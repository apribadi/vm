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
// #include "dasm.c"

typedef enum ThreadResult : U8 {
  THREAD_RESULT_OK,
  THREAD_RESULT_ABORT,
} ThreadResult;

struct Dom;

typedef ThreadResult (* OpF)(
  struct Dom *,
  U64 *,
  U64 *,
  L64 *,
  U64
);

typedef struct Dom {
  OpF ops[OP_COUNT];
  U64 buf[1024];
} Dom;

static inline ThreadResult dispatch(
    Dom * dp,
    U64 * sp,
    U64 * vp,
    L64 * ip,
    U64
) {
  U64 ic = PEEK_LE(U64, ip ++);
  TAIL return dp->ops[H0(ic)](dp, sp, vp, ip, ic);
}

static ThreadResult op_abort(Dom *, U64 *, U64 *, L64 *, U64) {
  return THREAD_RESULT_ABORT;
}

static ThreadResult op_branch(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  Bool p = PEEK(Bool, sp + H1(ic));
  S16 k = p ? (S16) H2(ic) : (S16) H3(ic);

  ip = ip - 1 + k;
  ic = PEEK_LE(U64, ip ++);
  vp = sp + H2(ic);

  assert(H0(ic) == OP_LABEL);
  assert(H1(ic) == 0);

  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_exit(Dom *, U64 *, U64 *, L64 *, U64) {
  return THREAD_RESULT_OK;
}

static ThreadResult op_jump(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  // H0     | H1     | H2     | H3
  // opcode | # args | offset | unused

  U16 n = H1(ic);
  S16 k = (S16) H2(ic);

  L64 * ap = ip;

  ip = ip - 1 + k;
  ic = PEEK_LE(U64, ip ++);
  vp = sp + H2(ic);

  assert(H0(ic) == OP_LABEL);
  assert(H1(ic) == n);

  if (n) {
    U64 * bp = dp->buf;
    U64 * cp = dp->buf;

    for (;;) {
      U64 x = PEEK_LE(U64, ap ++);
      ip ++;

      * bp ++ = sp[H0(x)];
      if (! -- n) break;
      * bp ++ = sp[H1(x)];
      if (! -- n) break;
      * bp ++ = sp[H2(x)];
      if (! -- n) break;
      * bp ++ = sp[H3(x)];
      if (! -- n) break;
    }

    while (cp != bp) {
      * vp ++ = * cp ++;
    }
  }

  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_nop(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_show_i64(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, sp + H1(ic));
  printf("%" PRIi64 "\n", x);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_const_i32(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  POKE(U32, vp ++, W1(ic));
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_const_i64(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  U64 x = PEEK_LE(U64, ip ++);
  POKE(U64, vp ++, x);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_f32_add(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  F32 x = PEEK(F32, sp + H1(ic));
  F32 y = PEEK(F32, sp + H2(ic));
  F32 z = x + y;
  POKE(F32, vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_f32_sqrt(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  F32 x = PEEK(F32, sp + H1(ic));
  F32 y = sqrtf(x);
  POKE(F32, vp ++, y);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_f64_add(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  F64 x = PEEK(F64, sp + H1(ic));
  F64 y = PEEK(F64, sp + H2(ic));
  F64 z = x + y;
  POKE(F64, vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_f64_sqrt(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  F64 x = PEEK(F64, sp + H1(ic));
  F64 y = sqrt(x);
  POKE(F64, vp ++, y);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_i32_add(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  U32 x = PEEK(U32, sp + H1(ic));
  U32 y = PEEK(U32, sp + H2(ic));
  U32 z = x + y;
  POKE(U32, vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_i64_add(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, sp + H1(ic));
  U64 y = PEEK(U64, sp + H2(ic));
  U64 z = x + y;
  POKE(U64, vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_i64_is_eq(Dom * dp, U64 * sp, U64 * vp, L64 * ip, U64 ic) {
  U64 x = PEEK(U64, sp + H1(ic));
  U64 y = PEEK(U64, sp + H2(ic));
  Bool z = x == y;
  POKE(Bool, vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult interpret(L64 * ip) {
  Dom dom = {
    .ops = {
      [OP_ABORT] = op_abort,
      [OP_BRANCH] = op_branch,
      [OP_EXIT] = op_exit,
      [OP_JUMP] = op_jump,
      [OP_NOP] = op_nop,
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
      [OP_I64_IS_EQ] = op_i64_is_eq,
    },
    .buf = { 0 },
  };

  U64 stack[256] = { 0 };

  return dispatch(&dom, &stack[0], &stack[0], ip, 0);
}

int main(int, char **) {
  L64 code[] = {
    /*  0 0 */ ic_make_h___(OP_CONST_I64),
    /*  1   */ ic_make_d___(10),
    /*  2 1 */ ic_make_h___(OP_CONST_I64),
    /*  3   */ ic_make_d___(0),
    /*  4 2 */ ic_make_hhh_(OP_I64_IS_EQ, 0, 1),
    /*  5   */ ic_make_hhhh(OP_BRANCH, 2, 1, 4),
    /*  6   */ ic_make_hhh_(OP_LABEL, 0, 3),
    /*  7   */ ic_make_hh__(OP_SHOW_I64, 1),
    /*  8   */ ic_make_h___(OP_EXIT),
    /*  9   */ ic_make_hhh_(OP_LABEL, 0, 3),
    /* 10 3 */ ic_make_h___(OP_CONST_I64),
    /* 11   */ ic_make_d___(1),
    /* 12   */ ic_make_hhh_(OP_JUMP, 3, 2),
    /* 13   */ ic_make_hhh_(3, 1, 3),
    /* 14 4 */ ic_make_hhh_(OP_LABEL, 3, 4),
    /* 15   */ ic_make_hhh_(TY_I64, TY_I64, TY_I64),
    /* 16 7 */ ic_make_hhh_(OP_I64_IS_EQ, 4, 0),
    /* 17   */ ic_make_hhhh(OP_BRANCH, 7, 6, 1),
    /* 18   */ ic_make_hhh_(OP_LABEL, 0, 8),
    /* 19 8 */ ic_make_hhh_(OP_I64_ADD, 5, 6),
    /* 20 9 */ ic_make_hhh_(OP_I64_ADD, 4, 3),
    /* 21   */ ic_make_hhh_(OP_JUMP, 3, (U16) -7),
    /* 22   */ ic_make_hhh_(9, 6, 8),
    /* 23   */ ic_make_hhh_(OP_LABEL, 0, 10),
    /* 24   */ ic_make_hh__(OP_SHOW_I64, 6),
    /* 25   */ ic_make_h___(OP_EXIT),
  };

  // disassemble(code, p);

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
