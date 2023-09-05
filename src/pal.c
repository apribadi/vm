#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prelude.c"
#include "code.c"
// #include "dasm.c"

typedef enum ThreadResult : U8 {
  THREAD_RESULT_OK,
  THREAD_RESULT_ABORT,
} ThreadResult;

struct OpTable;

typedef ThreadResult (* OpFun)(
  L64 *,
  U64 *,
  U64 *,
  struct OpTable *,
  L64
);

typedef struct OpTable {
  OpFun dispatch[OP_COUNT];
} OpTable;

STATIC_INLINE ThreadResult dispatch(
    L64 * ip,
    U64 * sp,
    U64 * vp,
    OpTable * tp,
    L64
) {
  L64 ic = * ip ++;
  TAIL return tp->dispatch[get_le_u16(&ic.h0)](ip, sp, vp, tp, ic);
}

STATIC_INLINE Bool var_bool(U64 * sp, L16 * ix) {
  return get_bool(&sp[get_le_u16(ix)]);
}

STATIC_INLINE U32 var_i32(U64 * sp, L16 * ix) {
  return get_u32(&sp[get_le_u16(ix)]);
}

STATIC_INLINE U64 var_i64(U64 * sp, L16 * ix) {
  return get_u64(&sp[get_le_u16(ix)]);
}

STATIC_INLINE F32 var_f32(U64 * sp, L16 * ix) {
  return get_f32(&sp[get_le_u16(ix)]);
}

STATIC_INLINE F64 var_f64(U64 * sp, L16 * ix) {
  return get_f64(&sp[get_le_u16(ix)]);
}

static ThreadResult op_abort(L64 *, U64 *, U64 *, OpTable *, L64) {
  return THREAD_RESULT_ABORT;
}

static ThreadResult op_branch(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  Bool p = var_bool(sp, &ic.h1);
  S16 k = p ? get_le_s16(&ic.h2) : get_le_s16(&ic.h3);

  ip = ip - 1 + k;
  ic = * ip ++;
  vp = sp + get_le_u16(&ic.h2);

  assert(get_le_u16(&ic.h0) == OP_LABEL);
  assert(get_le_u16(&ic.h1) == 0);

  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_exit(L64 *, U64 *, U64 *, OpTable *, L64) {
  return THREAD_RESULT_OK;
}

static U64 tmp[256];

static ThreadResult op_jump(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  U16 n = get_le_u16(&ic.h1); // jump arg count
  S16 k = get_le_s16(&ic.h2); // jump offset

  L64 * ap = ip;

  ip = ip - 1 + k;
  ic = * ip ++;
  vp = sp + get_le_u16(&ic.h2);

  assert(get_le_u16(&ic.h0) == OP_LABEL);
  assert(get_le_u16(&ic.h1) == n);

  if (n) {
    U64 * bp = tmp;
    U64 * cp = tmp;

    L64 x = * ap ++;
    L64 y = * ip ++;
    U16 i = 0;

    (void) y;

    for (;;) {
      * bp ++ = sp[get_le_u16(&x.h0)];
      if (++ i == n) break;
      * bp ++ = sp[get_le_u16(&x.h1)];
      if (++ i == n) break;
      * bp ++ = sp[get_le_u16(&x.h2)];
      if (++ i == n) break;
      * bp ++ = sp[get_le_u16(&x.h3)];
      if (++ i == n) break;

      x = * ap ++;

      * bp ++ = sp[get_le_u16(&x.h0)];
      if (++ i == n) break;
      * bp ++ = sp[get_le_u16(&x.h1)];
      if (++ i == n) break;
      * bp ++ = sp[get_le_u16(&x.h2)];
      if (++ i == n) break;
      * bp ++ = sp[get_le_u16(&x.h3)];
      if (++ i == n) break;

      x = * ap ++;
      y = * ip ++;
    }

    while (cp != bp) {
      * vp ++ = * cp ++;
    }
  }

  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_nop(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_show_i64(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  U64 x = var_i64(sp, &ic.h1);
  printf("%" PRIi64 "\n", x);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_const_f32(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  F32 x = get_le_f32(&ic.w1);
  set_f32(vp ++, x);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_const_f64(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  F64 x = get_le_f64(ip ++);
  set_f64(vp ++, x);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_const_i32(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  U32 x = get_le_u32(&ic.w1);
  set_u32(vp ++, x);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_const_i64(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  U64 x = get_le_u64(ip ++);
  set_u64(vp ++, x);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_prim_f32_add(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  F32 x = var_f32(sp, &ic.h1);
  F32 y = var_f32(sp, &ic.h2);
  F32 z = x + y;
  set_f32(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_prim_f32_sqrt(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  F32 x = var_f32(sp, &ic.h1);
  F32 y = sqrtf(x);
  set_f32(vp ++, y);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_prim_f64_add(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  F64 x = var_f64(sp, &ic.h1);
  F64 y = var_f64(sp, &ic.h2);
  F64 z = x + y;
  set_f64(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_prim_f64_sqrt(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  F64 x = var_f64(sp, &ic.h1);
  F64 y = sqrt(x);
  set_f64(vp ++, y);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_prim_i32_add(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  U32 x = var_i32(sp, &ic.h1);
  U32 y = var_i32(sp, &ic.h2);
  U32 z = x + y;
  set_u32(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_prim_i64_add(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  U64 x = var_i64(sp, &ic.h1);
  U64 y = var_i64(sp, &ic.h2);
  U64 z = x + y;
  set_u64(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static ThreadResult op_prim_i64_is_eq(L64 * ip, U64 * sp, U64 * vp, OpTable * tp, L64 ic) {
  U64 x = var_i64(sp, &ic.h1);
  U64 y = var_i64(sp, &ic.h2);
  Bool z = x == y;
  set_bool(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, ic);
}

static OpTable OP_TABLE = {
  .dispatch = {
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
};

static ThreadResult interpret(L64 * ip) {
  U64 stack[256] = { 0 };

  return dispatch(ip, &stack[0], &stack[0], &OP_TABLE, ic_make_d___(0));
}

int main(int, char **) {
  L64 code[256] = { 0 };

  L64 * p = &code[0];

  (void) p;

  /*  0 0 */ * p ++ = ic_make_h___(OP_CONST_I64);
  /*  1   */ * p ++ = ic_make_d___(10);
  /*  2 1 */ * p ++ = ic_make_h___(OP_CONST_I64);
  /*  3   */ * p ++ = ic_make_d___(0);
  /*  4 2 */ * p ++ = ic_make_hhh_(OP_PRIM_I64_IS_EQ, 0, 1);
  /*  5   */ * p ++ = ic_make_hhhh(OP_BRANCH, 2, 1, 4);
  /*  6   */ * p ++ = ic_make_hhh_(OP_LABEL, 0, 3);
  /*  7   */ * p ++ = ic_make_hh__(OP_SHOW_I64, 1);
  /*  8   */ * p ++ = ic_make_h___(OP_EXIT);
  /*  9   */ * p ++ = ic_make_hhh_(OP_LABEL, 0, 3);
  /* 10 3 */ * p ++ = ic_make_h___(OP_CONST_I64);
  /* 11   */ * p ++ = ic_make_d___(1);
  /* 12   */ * p ++ = ic_make_hhh_(OP_JUMP, 3, 2);
  /* 13   */ * p ++ = ic_make_hhh_(3, 1, 3);
  /* 14 4 */ * p ++ = ic_make_hhh_(OP_LABEL, 3, 4);
  /* 15   */ * p ++ = ic_make_bbbbbbbb(TY_I64, TY_I64, TY_I64, 0, 0, 0, 0, 0);
  /* 16 7 */ * p ++ = ic_make_hhh_(OP_PRIM_I64_IS_EQ, 4, 0);
  /* 17   */ * p ++ = ic_make_hhhh(OP_BRANCH, 7, 6, 1);
  /* 18   */ * p ++ = ic_make_hhh_(OP_LABEL, 0, 8);
  /* 19 8 */ * p ++ = ic_make_hhh_(OP_PRIM_I64_ADD, 5, 6);
  /* 20 9 */ * p ++ = ic_make_hhh_(OP_PRIM_I64_ADD, 4, 3);
  /* 21   */ * p ++ = ic_make_hhh_(OP_JUMP, 3, (U16) -7);
  /* 22   */ * p ++ = ic_make_hhh_(9, 6, 8);
  /* 23   */ * p ++ = ic_make_hhh_(OP_LABEL, 0, 10);
  /* 24   */ * p ++ = ic_make_hh__(OP_SHOW_I64, 6);
  /* 25   */ * p ++ = ic_make_h___(OP_EXIT);

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
