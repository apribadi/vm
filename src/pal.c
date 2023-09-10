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

typedef enum ThreadResult : U8 {
  THREAD_RESULT_OK,
  THREAD_RESULT_ABORT,
} ThreadResult;

struct Env;

typedef ThreadResult (* OpHandler)(
  struct Env *,
  U64 *,
  U64 *,
  U64 *,
  L64 *,
  U64
);

typedef struct Env {
  OpHandler dispatch[OP_COUNT];
} Env;

static inline ThreadResult dispatch(
    Env * ep,
    U64 * fp,
    U64 * vp,
    U64 * sp,
    L64 * ip,
    U64
) {
  U64 ic = PEEK_LE(U64, ip ++);
  TAIL return ep->dispatch[H0(ic)](ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_abort(Env *, U64 *, U64 *, U64 *, L64 *, U64) {
  return THREAD_RESULT_ABORT;
}

static ThreadResult op_call(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | CALL            | # args | # kont |              dst disp             |
  // | kont 0          | kont 1          |                 |                 |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // |                 |                 |                 |                 |
  // | ENTER           | # args | # kont | frame size      |                 |
  // | type 0          | type 1          | type 2          | type 3          |
  // |                 |                 |                 |                 |
  // | RET             | # args | kont   |                 |                 |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // |                 |                 |                 |                 |
  // | LABEL           | # args          | next var        |                 |
  // | type 0          | type 1          | type 2          | type 3          |

  // sp1 --> +-----+
  //         |     |  5
  //         |     |
  //         |     |  4
  // fp1 --> +-----+
  //         | ret |  3
  //         |     |
  //         | fp0 |  2
  // sp0 --> +-----+
  //         |     |  1
  //         |     |
  //         |     |  0
  // fp0 --> +-----+

  U64 * fp0 = fp;
  U64 * sp0 = sp;
  L64 * ip0 = ip;
  U64 ic0 = ic;

  L64 * ip1 = ip0 + (S32) W1(ic0) - 1;
  U64 ic1 = PEEK_LE(U64, ip1 ++);
  U64 * fp1 = sp0 + 2;
  U64 * vp1 = fp1;
  U64 * sp1 = fp1 + H2(ic1);

  // TODO: stack overflow check (w/ red zone)

  POKE(U64 *, sp0, fp0);
  POKE(L64 *, sp0 + 1, ip0 - 1);

  U8 an = B2(ic0); // argument number
  U8 kn = B3(ic0); // kontinuation number

  assert(H0(ic1) == OP_ENTER);
  assert(B2(ic1) == an);
  assert(B3(ic1) == kn);

  // skip kontinuations

  ip0 += ((U32) kn + 3) / 4;

  // pass args

  if (an) {
    for (;;) {
      U64 av = PEEK_LE(U64, ip0 ++); // argument variables
      ip1 ++;

      // TODO: if some variable has vector type, jump to a slow path handler
      // and redo the whole thing.

      * vp1 ++ = fp0[H0(av)]; if (! -- an) break;
      * vp1 ++ = fp0[H1(av)]; if (! -- an) break;
      * vp1 ++ = fp0[H2(av)]; if (! -- an) break;
      * vp1 ++ = fp0[H3(av)]; if (! -- an) break;
   }
  }

  fp = fp1;
  vp = vp1;
  sp = sp1;
  ip = ip1;
  ic = ic1;

  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_if(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | IF              | pred            | dst disp 0      | dst disp 0      |
  // |                 |                 |                 |                 |
  // | LABEL           | # args (= 0)    | next var        |                 |

  Bool p = PEEK(Bool, fp + H1(ic));
  S16 k = p ? (S16) H2(ic) : (S16) H3(ic);

  ip = ip - 1 + k;
  ic = PEEK_LE(U64, ip ++);
  vp = fp + H2(ic);

  assert(H0(ic) == OP_LABEL);
  assert(H1(ic) == 0);

  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_jump(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | JUMP            | # args          | dst disp        |                 |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // |                 |                 |                 |                 |
  // | LABEL           | # args          | next var        |                 |
  // | type 0          | type 1          | type 2          | type 3          |

  U16 n = H1(ic);
  S16 k = (S16) H2(ic);

  L64 * ap = ip;

  ip = ip - 1 + k;
  ic = PEEK_LE(U64, ip ++);
  vp = fp + H2(ic);

  assert(H0(ic) == OP_LABEL);
  assert(H1(ic) == n);

  if (n) {
    U64 * bp = sp; // RED ZONE
    U64 * cp = sp;

    for (;;) {
      U64 av = PEEK_LE(U64, ap ++); // argument variables
      ip ++;

      * bp ++ = fp[H0(av)]; if (! -- n) break;
      * bp ++ = fp[H1(av)]; if (! -- n) break;
      * bp ++ = fp[H2(av)]; if (! -- n) break;
      * bp ++ = fp[H3(av)]; if (! -- n) break;
    }

    while (cp != bp) {
      * vp ++ = * cp ++;
    }
  }

  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_nop(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_ret(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  /*
  U64 * fp0 = fp;
  U64 * sp0 = sp;
  L64 * ip0 = ip;
  U64 ic0 = ic;

  U64 * fp1 = PEEK(U64 *, fp0 - 2);
  U64 * vp1 = fp1;
  U64 * sp1 = fp0 - 2;
  L64 * ip1 = PEEK(L64 *, fp0 - 1);

  U8 an = B2(ic0); // argument number
  U8 ki = B3(ic0); // kontinuation index

  U64 ic1 = PEEK_LE(U64, ip1 ++);

  assert(H0(ic1) == OP_CALL);
  assert(B3(ic1) > ki);

  */
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_show_i64(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  printf("%" PRIi64 "\n", x);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_const_i32(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U32 x = W1(ic);
  POKE(U32, vp ++, x);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_const_i64(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK_LE(U64, ip ++);
  POKE(U64, vp ++, x);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_f32_add(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  F32 x = PEEK(F32, fp + H1(ic));
  F32 y = PEEK(F32, fp + H2(ic));
  POKE(F32, vp ++, x + y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_f32_sqrt(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  F32 x = PEEK(F32, fp + H1(ic));
  POKE(F32, vp ++, sqrtf(x));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_f64_add(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  F64 x = PEEK(F64, fp + H1(ic));
  F64 y = PEEK(F64, fp + H2(ic));
  POKE(F64, vp ++, x + y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_f64_sqrt(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  F64 x = PEEK(F64, fp + H1(ic));
  POKE(F64, vp ++, sqrt(x));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i32_add(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U32 x = PEEK(U32, fp + H1(ic));
  U32 y = PEEK(U32, fp + H2(ic));
  POKE(U32, vp ++, x + y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_add(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x + y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_bit_and(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x & y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_bit_not(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, ~ x);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_bit_or(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x | y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_bit_xor(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x ^ y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_byteswap(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, bswap64(x));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_clz(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, clz64(x));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_ctz(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, ctz64(x));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_eq(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x == y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_ge_s(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  S64 y = PEEK(S64, fp + H2(ic));
  POKE(Bool, vp ++, x >= y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_ge_u(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x >= y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_gt_s(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  S64 y = PEEK(S64, fp + H2(ic));
  POKE(Bool, vp ++, x > y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_gt_u(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x > y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_le_s(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  S64 y = PEEK(S64, fp + H2(ic));
  POKE(Bool, vp ++, x <= y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_le_u(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x <= y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_lt_s(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  S64 y = PEEK(S64, fp + H2(ic));
  POKE(Bool, vp ++, x < y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_lt_u(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x < y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_is_ne(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x != y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_mul(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x * y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_mul_full_s(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  S128 x = (S128) PEEK(S64, fp + H1(ic));
  S128 y = (S128) PEEK(S64, fp + H2(ic));
  S128 z = x * y;
  POKE(U64, vp ++, (U64) (U128) z);
  POKE(U64, vp ++, (U64) ((U128) z >> 64));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_mul_full_u(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U128 x = (U128) PEEK(U64, fp + H1(ic));
  U128 y = (U128) PEEK(U64, fp + H2(ic));
  U128 z = x * y;
  POKE(U64, vp ++, (U64) z);
  POKE(U64, vp ++, (U64) (z >> 64));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_mul_hi_s(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  S128 x = (S128) PEEK(S64, fp + H1(ic));
  S128 y = (S128) PEEK(S64, fp + H2(ic));
  S128 z = x * y;
  POKE(U64, vp ++, (U64) ((U128) z >> 64));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_mul_hi_u(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U128 x = (U128) PEEK(U64, fp + H1(ic));
  U128 y = (U128) PEEK(U64, fp + H2(ic));
  U128 z = x * y;
  POKE(U64, vp ++, (U64) (z >> 64));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_neg(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, - x);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_rol(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic)) & 0x3f;
  POKE(U64, vp ++, rol64(x, y));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_ror(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic)) & 0x3f;
  POKE(U64, vp ++, ror64(x, y));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_shl(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic)) & 0x3f;
  POKE(U64, vp ++, x << y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_shr_s(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  S64 x = (S64) PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic)) & 0x3f;
  POKE(U64, vp ++, (U64) (x >> y));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_shr_u(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic)) & 0x3f;
  POKE(U64, vp ++, x >> y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_sub(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x - y);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_to_f64_bits(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(F64, vp ++, PEEK(F64, &x));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_to_i32(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U32, vp ++, (U32) x);
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_to_i32_hi(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U32, vp ++, (U32) (x >> 32));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_to_i5(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U8, vp ++, (U8) (x & 0x1f));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult op_i64_to_i6(Env * ep, U64 * fp, U64 * vp, U64 * sp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U8, vp ++, (U8) (x & 0x3f));
  TAIL return dispatch(ep, fp, vp, sp, ip, ic);
}

static ThreadResult interpret(L64 * ip) {
  Env dom = {
    .dispatch = {
      [OP_ABORT] = op_abort,
      [OP_CALL] = op_call,
      [OP_IF] = op_if,
      [OP_JUMP] = op_jump,
      [OP_NOP] = op_nop,
      [OP_RET] = op_ret,
      [OP_SHOW_I64] = op_show_i64,
      [OP_CONST_F32] = op_const_i32, // same as OP_CONST_I32
      [OP_CONST_F64] = op_const_i64, // same as OP_CONST_I64
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
      [OP_I64_BYTESWAP] = op_i64_byteswap,
      [OP_I64_CLZ] = op_i64_clz,
      [OP_I64_CTZ] = op_i64_ctz,
      [OP_I64_IS_EQ] = op_i64_is_eq,
      [OP_I64_IS_GE_S] = op_i64_is_ge_s,
      [OP_I64_IS_GE_U] = op_i64_is_ge_u,
      [OP_I64_IS_GT_S] = op_i64_is_gt_s,
      [OP_I64_IS_GT_U] = op_i64_is_gt_u,
      [OP_I64_IS_LE_S] = op_i64_is_le_s,
      [OP_I64_IS_LE_U] = op_i64_is_le_u,
      [OP_I64_IS_LT_S] = op_i64_is_lt_s,
      [OP_I64_IS_LT_U] = op_i64_is_lt_u,
      [OP_I64_IS_NE] = op_i64_is_ne,
      [OP_I64_MUL] = op_i64_mul,
      [OP_I64_MUL_FULL_S] = op_i64_mul_full_s,
      [OP_I64_MUL_FULL_U] = op_i64_mul_full_u,
      [OP_I64_MUL_HI_S] = op_i64_mul_hi_s,
      [OP_I64_MUL_HI_U] = op_i64_mul_hi_u,
      [OP_I64_NEG] = op_i64_neg,
      [OP_I64_ROL] = op_i64_rol,
      [OP_I64_ROR] = op_i64_ror,
      [OP_I64_SHL] = op_i64_shl,
      [OP_I64_SHR_S] = op_i64_shr_s,
      [OP_I64_SHR_U] = op_i64_shr_u,
      [OP_I64_SUB] = op_i64_sub,
      [OP_I64_TO_F64_BITS] = op_i64_to_f64_bits,
      [OP_I64_TO_I32] = op_i64_to_i32,
      [OP_I64_TO_I32_HI] = op_i64_to_i32_hi,
      [OP_I64_TO_I5] = op_i64_to_i5,
      [OP_I64_TO_I6] = op_i64_to_i6,
    },
  };

  U64 stack[256] = { 0 };

  return dispatch(&dom, &stack[0], &stack[0], &stack[128], ip, 0);
}

int main(int, char **) {
  L64 code[] = {
    /*  0 0 */ ic_make_h___(OP_CONST_I64),
    /*  1   */ ic_make_d___(10),
    /*  2 1 */ ic_make_h___(OP_CONST_I64),
    /*  3   */ ic_make_d___(0),
    /*  4 2 */ ic_make_hhh_(OP_I64_IS_EQ, 0, 1),
    /*  5   */ ic_make_hhhh(OP_IF, 2, 1, 4),
    /*  6   */ ic_make_hhh_(OP_LABEL, 0, 3),
    /*  7   */ ic_make_hh__(OP_SHOW_I64, 1),
    /*  8   */ ic_make_h___(OP_ABORT),
    /*  9   */ ic_make_hhh_(OP_LABEL, 0, 3),
    /* 10 3 */ ic_make_h___(OP_CONST_I64),
    /* 11   */ ic_make_d___(1),
    /* 12   */ ic_make_hhh_(OP_JUMP, 3, 2),
    /* 13   */ ic_make_hhh_(3, 1, 3),
    /* 14 4 */ ic_make_hhh_(OP_LABEL, 3, 4),
    /* 15   */ ic_make_hhh_(TY_I64, TY_I64, TY_I64),
    /* 16 7 */ ic_make_hhh_(OP_I64_IS_EQ, 4, 0),
    /* 17   */ ic_make_hhhh(OP_IF, 7, 6, 1),
    /* 18   */ ic_make_hhh_(OP_LABEL, 0, 8),
    /* 19 8 */ ic_make_hhh_(OP_I64_ADD, 5, 6),
    /* 20 9 */ ic_make_hhh_(OP_I64_ADD, 4, 3),
    /* 21   */ ic_make_hhh_(OP_JUMP, 3, (U16) -7),
    /* 22   */ ic_make_hhh_(9, 6, 8),
    /* 23   */ ic_make_hhh_(OP_LABEL, 0, 10),
    /* 24   */ ic_make_hh__(OP_SHOW_I64, 6),
    /* 25   */ ic_make_h___(OP_ABORT),
  };

  disassemble(code, code + sizeof(code) / sizeof(L64));

  ThreadResult r = interpret(&code[0]);

  switch (r) {
    case THREAD_RESULT_OK:
      printf("ok\n");
      break;
    case THREAD_RESULT_ABORT:
      printf("abort!\n");
      break;
  }

  return 0;
}
