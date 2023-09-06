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
  L64
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
    L64
) {
  L64 ic = * ip ++;
  TAIL return dp->ops[get_le_u16(&ic.h0)](dp, sp, vp, ip, ic);
}

static inline Bool var_bool(U64 * sp, L16 * ix) {
  return get_bool(&sp[get_le_u16(ix)]);
}

static inline U32 var_i32(U64 * sp, L16 * ix) {
  return get_u32(&sp[get_le_u16(ix)]);
}

static inline U64 var_i64(U64 * sp, L16 * ix) {
  return get_u64(&sp[get_le_u16(ix)]);
}

static inline F32 var_f32(U64 * sp, L16 * ix) {
  return get_f32(&sp[get_le_u16(ix)]);
}

static inline F64 var_f64(U64 * sp, L16 * ix) {
  return get_f64(&sp[get_le_u16(ix)]);
}

static ThreadResult op_abort(Dom *, U64 *, U64 *, L64 *, L64) {
  return THREAD_RESULT_ABORT;
}

static ThreadResult op_branch(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  Bool p = var_bool(sp, &ic.h1);
  S16 k = p ? get_le_s16(&ic.h2) : get_le_s16(&ic.h3);

  ip = ip - 1 + k;
  ic = * ip ++;
  vp = sp + get_le_u16(&ic.h2);

  assert(get_le_u16(&ic.h0) == OP_LABEL);
  assert(get_le_u16(&ic.h1) == 0);

  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_exit(Dom *, U64 *, U64 *, L64 *, L64) {
  return THREAD_RESULT_OK;
}

static ThreadResult op_jump(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  U8 n = get_le_u8(&ic.h1); // jump arg count
  S16 k = get_le_s16(&ic.h2); // jump offset

  L64 * ap = ip;

  ip = ip - 1 + k;
  ic = * ip ++;
  vp = sp + get_le_u16(&ic.h2);

  assert(get_le_u16(&ic.h0) == OP_LABEL);
  assert(get_le_u16(&ic.h1) == n);

  if (n) {
    U64 * bp = dp->buf;
    U64 * cp = dp->buf;

    for (;;) {
      L64 x = * ap ++;
      ip ++;

      * bp ++ = sp[get_le_u16(&x.h0)];
      if (! -- n) break;
      * bp ++ = sp[get_le_u16(&x.h1)];
      if (! -- n) break;
      * bp ++ = sp[get_le_u16(&x.h2)];
      if (! -- n) break;
      * bp ++ = sp[get_le_u16(&x.h3)];
      if (! -- n) break;
    }

    while (cp != bp) {
      * vp ++ = * cp ++;
    }
  }

  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_nop(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_show_i64(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  U64 x = var_i64(sp, &ic.h1);
  printf("%" PRIi64 "\n", x);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_const_f32(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  F32 x = get_le_f32(&ic.w1);
  set_f32(vp ++, x);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_const_f64(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  F64 x = get_le_f64(ip ++);
  set_f64(vp ++, x);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_const_i32(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  U32 x = get_le_u32(&ic.w1);
  set_u32(vp ++, x);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_const_i64(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  U64 x = get_le_u64(ip ++);
  set_u64(vp ++, x);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_prim_f32_add(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  F32 x = var_f32(sp, &ic.h1);
  F32 y = var_f32(sp, &ic.h2);
  F32 z = x + y;
  set_f32(vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_prim_f32_sqrt(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  F32 x = var_f32(sp, &ic.h1);
  F32 y = sqrtf(x);
  set_f32(vp ++, y);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_prim_f64_add(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  F64 x = var_f64(sp, &ic.h1);
  F64 y = var_f64(sp, &ic.h2);
  F64 z = x + y;
  set_f64(vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_prim_f64_sqrt(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  F64 x = var_f64(sp, &ic.h1);
  F64 y = sqrt(x);
  set_f64(vp ++, y);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_prim_i32_add(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  U32 x = var_i32(sp, &ic.h1);
  U32 y = var_i32(sp, &ic.h2);
  U32 z = x + y;
  set_u32(vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_prim_i64_add(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  U64 x = var_i64(sp, &ic.h1);
  U64 y = var_i64(sp, &ic.h2);
  U64 z = x + y;
  set_u64(vp ++, z);
  TAIL return dispatch(dp, sp, vp, ip, ic);
}

static ThreadResult op_prim_i64_is_eq(Dom * dp, U64 * sp, U64 * vp, L64 * ip, L64 ic) {
  U64 x = var_i64(sp, &ic.h1);
  U64 y = var_i64(sp, &ic.h2);
  Bool z = x == y;
  set_bool(vp ++, z);
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
      [OP_CONST_F32] = op_const_f32,
      [OP_CONST_F64] = op_const_f64,
      [OP_CONST_I32] = op_const_i32,
      [OP_CONST_I64] = op_const_i64,
      [OP_PRIM_F32_ADD] = op_prim_f32_add,
      [OP_PRIM_F32_SQRT] = op_prim_f32_sqrt,
      [OP_PRIM_F64_ADD] = op_prim_f64_add,
      [OP_PRIM_F64_SQRT] = op_prim_f64_sqrt,
      [OP_PRIM_I32_ADD] = op_prim_i32_add,
      [OP_PRIM_I64_ADD] = op_prim_i64_add,
      [OP_PRIM_I64_IS_EQ] = op_prim_i64_is_eq,
    },
    .buf = { 0 },
  };

  U64 stack[256] = { 0 };

  return dispatch(&dom, &stack[0], &stack[0], ip, ic_make_d___(0));
}

int main(int, char **) {
  L64 code[] = {
    /*  0 0 */ ic_make_h___(OP_CONST_I64),
    /*  1   */ ic_make_d___(10),
    /*  2 1 */ ic_make_h___(OP_CONST_I64),
    /*  3   */ ic_make_d___(0),
    /*  4 2 */ ic_make_hhh_(OP_PRIM_I64_IS_EQ, 0, 1),
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
    /* 16 7 */ ic_make_hhh_(OP_PRIM_I64_IS_EQ, 4, 0),
    /* 17   */ ic_make_hhhh(OP_BRANCH, 7, 6, 1),
    /* 18   */ ic_make_hhh_(OP_LABEL, 0, 8),
    /* 19 8 */ ic_make_hhh_(OP_PRIM_I64_ADD, 5, 6),
    /* 20 9 */ ic_make_hhh_(OP_PRIM_I64_ADD, 4, 3),
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
