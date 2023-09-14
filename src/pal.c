#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAIL __attribute__((musttail))

#include "prelude.c"
#include "code.c"
#include "dasm.c"

typedef enum ExitCode : U32 {
  EXIT_CODE_OK,
  EXIT_CODE_PANIC,
} ExitCode;

struct Env;

typedef ExitCode (* OpHandler)(
  struct Env *,
  L64 *,
  U64 *,
  U64 *,
  U64 *,
  U64
);

typedef struct Env {
  OpHandler dispatch[OP_COUNT];
} Env;

static inline ExitCode dispatch(
    Env * ep,
    L64 * ip,
    U64 * fp,
    U64 * vp,
    U64 * sp,
    U64
) {
  U64 ic = PEEK_LE(U64, ip ++);
  TAIL return ep->dispatch[H0(ic)](ep, ip, fp, vp, sp, ic);
}

static ExitCode op_abort(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  (void) ep;
  (void) ip;
  (void) fp;
  (void) vp;
  (void) sp;

  return W1(ic);
}

static ExitCode op_call(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | CALL            | # args | # kont |              dst disp             |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // | kont disp 0     | kont disp 1     |                 |                 |
  // |                 |                 |                 |                 |
  // | CALL_INDIRECT   | # args | # kont | fun ptr var     |                 |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // | kont disp 0     | kont disp 1     |                 |                 |
  //
  // | ENTER           | # args | # kont | frame size      |                 |
  // | param type 0    | param type 1    | param type 2    | param type 3    |

  //         sp1 --> +-----+
  //                 | xxx |
  //                 | xxx |
  //                 +-----+
  //                 |     |
  //                 |     |
  // sp0 --> fp1 --> +-----+
  //                 | ret |
  //                 | fp0 |
  //                 +-----+
  //                 |     |
  //                 |     |
  // fp0 ----------> +-----+

  L64 * ip0 = ip;
  U64 * fp0 = fp;
  U64 * sp0 = sp;

  U16 an = (U16) B2(ic); // # arguments
  U16 kn = (U16) B3(ic); // # kontinuations

  (void) vp;
  (void) kn;

  // store frame pointer and return address

  POKE(U64 *, sp0 - 2, fp0);
  POKE(L64 *, sp0 - 1, ip0 - 1);

  // enter function

  switch (H0(ic)) {
    case OP_CALL:
      ip = ip0 - 1 + (S32) W1(ic);
      break;
    case OP_CALL_INDIRECT:
      ip = PEEK(L64 *, fp0 + H2(ic));
      break;
    default:
      __builtin_unreachable();
  }

  ic = PEEK_LE(U64, ip ++);

  assert(H0(ic) == OP_ENTER);
  assert(B2(ic) == an);
  assert(B3(ic) == kn);

  // make new stack frame

  fp = sp0;
  vp = sp0;
  sp = sp0 + H2(ic) + 2;

  // TODO: stack overflow check

  // copy args

  if (an) {
    while (true) {
      U64 ic0 = PEEK_LE(U64, ip0 ++);
      ip ++;

      * vp ++ = fp0[H0(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H1(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H2(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H3(ic0)]; if (! -- an) break;
   }
  }

  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_if(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | IF              | pred            | dst disp 0      | dst disp 0      |
  // |                 |                 |                 |                 |
  // | LABEL           | # args (= 0)    | next var        |                 |

  Bool p = PEEK(Bool, fp + H1(ic));
  S16 a = (S16) H2(ic);
  S16 b = (S16) H3(ic);
  S16 di = p ? a : b;

  ip = ip - 1 + di;
  ic = PEEK_LE(U64, ip ++);
  vp = fp + H2(ic);

  assert(H0(ic) == OP_LABEL);
  assert(H1(ic) == 0);

  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_jump(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | JUMP            | # args          | dst disp        |                 |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // |                 |                 |                 |                 |
  // | LABEL           | # args          | next var        |                 |
  // | param type 0    | param type 1    | param type 2    | param type 3    |

  L64 * ip0 = ip;

  U16 an = H1(ic);
  S16 di = (S16) H2(ic);

  ip = ip - 1 + di;
  ic = PEEK_LE(U64, ip ++);
  vp = fp + H2(ic);

  assert(H0(ic) == OP_LABEL);
  assert(H1(ic) == an);

  if (an) {
    // TODO: stack overflow check

    U64 * ap = sp;
    U64 * bp = sp;

    // copy args into scratch space

    while (true) {
      U64 ic0 = PEEK_LE(U64, ip0 ++);
      ip ++;

      * ap ++ = fp[H0(ic0)]; if (! -- an) break;
      * ap ++ = fp[H1(ic0)]; if (! -- an) break;
      * ap ++ = fp[H2(ic0)]; if (! -- an) break;
      * ap ++ = fp[H3(ic0)]; if (! -- an) break;
    }

    // copy args from scratch space

    while (bp != ap) {
      * vp ++ = * bp ++;
    }
  }

  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_nop(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_ret(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | RET             | # args | kont   |                 |                 |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // |                 |                 |                 |                 |
  // | CALL            | # args | # kont |              dst disp             |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // | kont disp 0     | kont disp 1     |                 |                 |
  // |                 |                 |                 |                 |
  // | LABEL           | # params        | next var        |                 |
  // | param type 0    | param type 1    | param type 0    | param type 3    |

  // sp0 ----------> +-----+
  //                 | xxx |
  //                 | xxx |
  //                 +-----+
  //                 |     |
  //                 |     |
  // fp0 --> sp1 --> +-----+
  //                 | ret |
  //                 | fp1 |
  //                 +-----+
  //                 |     |
  //                 |     |
  //         fp1 --> +-----+

  L64 * ip0 = ip;
  U64 * fp0 = fp;

  U16 an = (U16) B2(ic);
  U16 ki = (U16) B3(ic);

  fp = PEEK(U64 *, fp0 - 2);
  ip = PEEK(L64 *, fp0 - 1);
  sp = fp0;
  ic = PEEK_LE(U64, ip ++);

  assert(
      H0(ic) == OP_CALL
      || H0(ic) == OP_CALL_INDIRECT
      || H0(ic) == OP_CALL_INDIRECT_TAIL
      || H0(ic) == OP_CALL_TAIL
    );

  assert(ki < B3(ic));

  S16 di = (S16) H_(PEEK_LE(U64, ip + (B2(ic) + 3) / 4 + ki / 4), ki & 3);

  ip = ip - 1 + di;
  ic = PEEK_LE(U64, ip ++);
  vp = fp + H2(ic);

  assert(H0(ic) == OP_LABEL);
  assert(H1(ic) == an);

  if (an) {
    while (true) {
      U64 ic0 = PEEK_LE(U64, ip0 ++);
      ip ++;

      * vp ++ = fp0[H0(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H1(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H2(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H3(ic0)]; if (! -- an) break;
   }
  }

  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_show_i64(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  printf("%" PRIi64 "\n", x);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_const_f32(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U32 x = W1(ic);
  F32 y = PEEK(F32, &x);
  POKE(F32, vp ++, y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_const_f64(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK_LE(U64, ip ++);
  F64 y = PEEK(F64, &x);
  POKE(F64, vp ++, y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_const_i32(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U32 x = W1(ic);
  POKE(U32, vp ++, x);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_const_i64(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK_LE(U64, ip ++);
  POKE(U64, vp ++, x);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_f32_add(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  F32 x = PEEK(F32, fp + H1(ic));
  F32 y = PEEK(F32, fp + H2(ic));
  POKE(F32, vp ++, x + y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_f32_sqrt(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  F32 x = PEEK(F32, fp + H1(ic));
  POKE(F32, vp ++, sqrtf(x));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_f64_add(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  F64 x = PEEK(F64, fp + H1(ic));
  F64 y = PEEK(F64, fp + H2(ic));
  POKE(F64, vp ++, x + y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_f64_sqrt(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  F64 x = PEEK(F64, fp + H1(ic));
  POKE(F64, vp ++, sqrt(x));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i32_add(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U32 x = PEEK(U32, fp + H1(ic));
  U32 y = PEEK(U32, fp + H2(ic));
  POKE(U32, vp ++, x + y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_add(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x + y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_bit_and(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x & y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_bit_not(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, ~ x);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_bit_or(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x | y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_bit_xor(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x ^ y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_clz(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, (U64) clz64(x));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_ctz(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, (U64) ctz64(x));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_is_eq(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x == y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_is_le_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  S64 y = PEEK(S64, fp + H2(ic));
  POKE(Bool, vp ++, x <= y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_is_le_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x <= y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_is_lt_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  S64 y = PEEK(S64, fp + H2(ic));
  POKE(Bool, vp ++, x < y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_is_lt_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x < y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_mul(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x * y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_mul_full_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S128 x = (S128) PEEK(S64, fp + H1(ic));
  S128 y = (S128) PEEK(S64, fp + H2(ic));
  S128 z = x * y;
  POKE(U64, vp ++, (U64) (U128) z);
  POKE(U64, vp ++, (U64) ((U128) z >> 64));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_mul_full_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U128 x = (U128) PEEK(U64, fp + H1(ic));
  U128 y = (U128) PEEK(U64, fp + H2(ic));
  U128 z = x * y;
  POKE(U64, vp ++, (U64) z);
  POKE(U64, vp ++, (U64) (z >> 64));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_mul_hi_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S128 x = (S128) PEEK(S64, fp + H1(ic));
  S128 y = (S128) PEEK(S64, fp + H2(ic));
  S128 z = x * y;
  POKE(U64, vp ++, (U64) ((U128) z >> 64));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_mul_hi_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U128 x = (U128) PEEK(U64, fp + H1(ic));
  U128 y = (U128) PEEK(U64, fp + H2(ic));
  U128 z = x * y;
  POKE(U64, vp ++, (U64) (z >> 64));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_neg(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, - x);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_rev(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, rev64(x));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_rol(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(U64, vp ++, rol64(x, y));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_ror(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(U64, vp ++, ror64(x, y));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_select(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  Bool p = PEEK(Bool, fp + H1(ic));
  U16 i = p ? H2(ic) : H3(ic);
  U64 x = PEEK(U64, fp + i);
  POKE(U64, vp ++, x);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_shl(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(U64, vp ++, shl64(x, y));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_shr_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(S64, vp ++, asr64(x, y));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_shr_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(U64, vp ++, lsr64(x, y));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_sub(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x - y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_to_f64_bitcast(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  F64 y = PEEK(F64, &x);
  POKE(F64, vp ++, y);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_to_i32(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U32, vp ++, (U32) x);
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_to_i32_hi(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U32, vp ++, (U32) (x >> 32));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_to_i5(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U8, vp ++, (U8) (x & 0x1f));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode op_i64_to_i6(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U8, vp ++, (U8) (x & 0x3f));
  TAIL return dispatch(ep, ip, fp, vp, sp, ic);
}

static ExitCode interpret(L64 * ip) {
  Env dom = {
    .dispatch = {
      [OP_ABORT] = op_abort,
      [OP_CALL] = op_call,
      [OP_CALL_INDIRECT] = op_call,
      [OP_IF] = op_if,
      [OP_JUMP] = op_jump,
      [OP_NOP] = op_nop,
      [OP_RET] = op_ret,
      [OP_SHOW_I64] = op_show_i64,
      [OP_CONST_F32] = op_const_f32,
      [OP_CONST_F64] = op_const_f64,
      [OP_CONST_I32] = op_const_i32,
      [OP_CONST_I64] = op_const_i64,
      [OP_F32_ADD] = op_f32_add,
      [OP_F32_SQRT] = op_f32_sqrt,
      [OP_F64_ADD] = op_f64_add,
      [OP_F64_SQRT] = op_f64_sqrt,
      [OP_I32_ADD] = op_i32_add,
      [OP_I64_ADD] = op_i64_add,
      [OP_I64_BIT_AND] = op_i64_bit_and,
      [OP_I64_BIT_NOT] = op_i64_bit_not,
      [OP_I64_BIT_OR] = op_i64_bit_or,
      [OP_I64_BIT_XOR] = op_i64_bit_xor,
      [OP_I64_CLZ] = op_i64_clz,
      [OP_I64_CTZ] = op_i64_ctz,
      [OP_I64_IS_EQ] = op_i64_is_eq,
      [OP_I64_IS_LE_S] = op_i64_is_le_s,
      [OP_I64_IS_LE_U] = op_i64_is_le_u,
      [OP_I64_IS_LT_S] = op_i64_is_lt_s,
      [OP_I64_IS_LT_U] = op_i64_is_lt_u,
      [OP_I64_MUL] = op_i64_mul,
      [OP_I64_MUL_FULL_S] = op_i64_mul_full_s,
      [OP_I64_MUL_FULL_U] = op_i64_mul_full_u,
      [OP_I64_MUL_HI_S] = op_i64_mul_hi_s,
      [OP_I64_MUL_HI_U] = op_i64_mul_hi_u,
      [OP_I64_NEG] = op_i64_neg,
      [OP_I64_REV] = op_i64_rev,
      [OP_I64_ROL] = op_i64_rol,
      [OP_I64_ROR] = op_i64_ror,
      [OP_I64_SELECT] = op_i64_select,
      [OP_I64_SHL] = op_i64_shl,
      [OP_I64_SHR_S] = op_i64_shr_s,
      [OP_I64_SHR_U] = op_i64_shr_u,
      [OP_I64_SUB] = op_i64_sub,
      [OP_I64_TO_F64_BITCAST] = op_i64_to_f64_bitcast,
      [OP_I64_TO_I32] = op_i64_to_i32,
      [OP_I64_TO_I32_HI] = op_i64_to_i32_hi,
      [OP_I64_TO_I5] = op_i64_to_i5,
      [OP_I64_TO_I6] = op_i64_to_i6,
    },
  };

  U64 stack[256] = { 0 };

  L64 stub[] = {
    ic_make_hbw_(OP_CALL, 0, 1, /* dummy */ 0),
    ic_make_h___(2),
    ic_make_hhh_(OP_LABEL, 0, 0),
    ic_make_h_w_(OP_ABORT, EXIT_CODE_OK),
  };

  U64 ic = PEEK_LE(U64, ip ++);

  assert(H0(ic) == OP_ENTER);
  assert(B2(ic) == 0);
  assert(B3(ic) == 1);

  POKE(U64 *, &stack[0], &stack[0]);
  POKE(L64 *, &stack[1], &stub[0]);

  return
    dispatch(
        &dom,
        ip,
        &stack[2],
        &stack[2],
        &stack[2 + H2(ic) + 2],
        ic
    );
}

int main(int, char **) {
  L64 code[] = {
    /*   */ ic_make_hbh_(OP_ENTER, 0, 1, 2),
    /* 0 */ ic_make_h___(OP_CONST_I64),
    /*   */ ic_make_d___(10),
    /*   */ ic_make_hbw_(OP_CALL, 1, 1, 7),
    /*   */ ic_make_h___(0),
    /*   */ ic_make_h___(3),
    /* 1 */ ic_make_hhh_(OP_LABEL, 1, 1),
    /*   */ ic_make_h___(TY_I64),
    /*   */ ic_make_hh__(OP_SHOW_I64, 1),
    /*   */ ic_make_hb__(OP_RET, 0, 0),
    /* 0 */ ic_make_hbh_(OP_ENTER, 1, 1, 10),
    /*   */ ic_make_h___(TY_I64),
    /* 1 */ ic_make_h___(OP_CONST_I64),
    /*   */ ic_make_d___(0),
    /* 2 */ ic_make_hhh_(OP_I64_IS_EQ, 0, 1),
    /*   */ ic_make_hhhh(OP_IF, 2, 1, 4),
    /*   */ ic_make_hhh_(OP_LABEL, 0, 3),
    /*   */ ic_make_hb__(OP_RET, 1, 0),
    /*   */ ic_make_h___(1),
    /*   */ ic_make_hhh_(OP_LABEL, 0, 3),
    /* 3 */ ic_make_h___(OP_CONST_I64),
    /*   */ ic_make_d___(1),
    /*   */ ic_make_hhh_(OP_JUMP, 3, 2),
    /*   */ ic_make_hhh_(3, 1, 3),
    /* 4 */ ic_make_hhh_(OP_LABEL, 3, 4),
    /*   */ ic_make_hhh_(TY_I64, TY_I64, TY_I64),
    /* 7 */ ic_make_hhh_(OP_I64_IS_EQ, 4, 0),
    /*   */ ic_make_hhhh(OP_IF, 7, 6, 1),
    /*   */ ic_make_hhh_(OP_LABEL, 0, 8),
    /* 8 */ ic_make_hhh_(OP_I64_ADD, 5, 6),
    /* 9 */ ic_make_hhh_(OP_I64_ADD, 4, 3),
    /*   */ ic_make_hhh_(OP_JUMP, 3, (U16) -7),
    /*   */ ic_make_hhh_(9, 6, 8),
    /*   */ ic_make_hhh_(OP_LABEL, 0, 10),
    /*   */ ic_make_hb__(OP_RET, 1, 0),
    /*   */ ic_make_h___(6),
  };

  disassemble(code, code + sizeof(code) / sizeof(L64));

  ExitCode r = interpret(&code[0]);

  switch (r) {
    case EXIT_CODE_OK:
      printf("ok\n");
      break;
    case EXIT_CODE_PANIC:
      printf("panic!\n");
      break;
  }

  return 0;
}
