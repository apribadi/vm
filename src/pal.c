#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "prelude.c"
#include "code.c"
#include "dasm.c"

enum ThreadResult: u8 {
  THREAD_RESULT_OK,
  THREAD_RESULT_ABORT,
};

struct OpTable;

typedef enum ThreadResult (* OpFun)(
  u64 *,
  u64 *,
  u64 *,
  struct OpTable *,
  u64
);

struct OpTable {
  OpFun dispatch[OP_COUNT];
};

STATIC_INLINE enum ThreadResult dispatch(
    u64 * ip,
    u64 * sp,
    u64 * vp,
    struct OpTable * tp,
    u64 old_iw
) {
  (void) old_iw;

  u64 iw = * ip ++;
  TAIL return tp->dispatch[iw_b0(iw)](ip, sp, vp, tp, iw);
}

STATIC_INLINE bool var_bool(u64 * sp, u16 ix) {
  return (u8) sp[ix];
}

STATIC_INLINE u32 var_i32(u64 * sp, u16 ix) {
  return (u32) sp[ix];
}

STATIC_INLINE u64 var_i64(u64 * sp, u16 ix) {
  return sp[ix];
}

STATIC_INLINE f32 var_f32(u64 * sp, u16 ix) {
  return bitcast_u32_to_f32((u32) sp[ix]);
}

STATIC_INLINE f64 var_f64(u64 * sp, u16 ix) {
  return bitcast_u64_to_f64(sp[ix]);
}

static enum ThreadResult op_abort(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  (void) ip;
  (void) sp;
  (void) vp;
  (void) tp;
  (void) iw;

  return THREAD_RESULT_ABORT;
}

static enum ThreadResult op_exit(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  (void) ip;
  (void) sp;
  (void) vp;
  (void) tp;
  (void) iw;

  return THREAD_RESULT_OK;
}

static enum ThreadResult op_branch(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  bool p = var_bool(sp, iw_b1(iw));
  s16 k = p ? (s16) iw_b2(iw) : (s16) iw_b3(iw);

  ip = ip - 1 + k;
  iw = * ip ++;
  vp = sp + iw_b2(iw);

  assert(iw_b0(iw) == OP_LABEL);
  assert(iw_b1(iw) == 0);

  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_nop(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_show_i64(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  u64 x = var_i64(sp, iw_b1(iw));
  printf("%" PRIi64 "\n", x);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_const_i32(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  * vp ++ = iw_c1(iw);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_const_i64(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  * vp ++ = * ip ++;
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_const_v256(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  * vp ++ = * ip ++;
  * vp ++ = * ip ++;
  * vp ++ = * ip ++;
  * vp ++ = * ip ++;
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_f32_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  f32 x = var_f32(sp, iw_b1(iw));
  f32 y = var_f32(sp, iw_b2(iw));
  f32 z = x + y;
  * vp ++ = bitcast_f32_to_u32(z);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_f32_sqrt(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  f32 x = var_f32(sp, iw_b1(iw));
  f32 y = sqrtf(x);
  * vp ++ = bitcast_f32_to_u32(y);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_f64_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  f64 x = var_f64(sp, iw_b1(iw));
  f64 y = var_f64(sp, iw_b2(iw));
  f64 z = x + y;
  * vp ++ = bitcast_f64_to_u64(z);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_f64_sqrt(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  f64 x = var_f64(sp, iw_b1(iw));
  f64 y = sqrt(x);
  * vp ++ = bitcast_f64_to_u64(y);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_i32_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  u32 x = var_i32(sp, iw_b1(iw));
  u32 y = var_i32(sp, iw_b2(iw));
  u32 z = x + y;
  * vp ++ = z;
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_i64_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 iw) {
  u64 x = var_i64(sp, iw_b1(iw));
  u64 y = var_i64(sp, iw_b2(iw));
  u64 z = x + y;
  * vp ++ = z;
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static struct OpTable OP_TABLE = {
  .dispatch = {
    [OP_ABORT] = op_abort,
    [OP_BRANCH] = op_branch,
    [OP_EXIT] = op_exit,
    [OP_NOP] = op_nop,
    [OP_SHOW_I64] = op_show_i64,
    [OP_CONST_F32] = op_const_i32, // same as op_const_i32
    [OP_CONST_F64] = op_const_i64, // same as op_const_i64
    [OP_CONST_I32] = op_const_i32,
    [OP_CONST_I64] = op_const_i64,
    [OP_CONST_V256] = op_const_v256,
    [OP_PRIM_F32_ADD] = op_prim_f32_add,
    [OP_PRIM_F32_SQRT] = op_prim_f32_sqrt,
    [OP_PRIM_F64_ADD] = op_prim_f64_add,
    [OP_PRIM_I32_ADD] = op_prim_i32_add,
    [OP_PRIM_I64_ADD] = op_prim_i64_add,
  },
};

static enum ThreadResult interpret(u64 * ip) {
  u64 stack[256];

  return dispatch(ip, &stack[0], &stack[0], &OP_TABLE, 0);
}

int main(int argc, char ** argv) {
  (void) argc;
  (void) argv;

  u64 code[256] = { 0 };

  u64 * p = &code[0];

  * p ++ = iw_make_o___(OP_CONST_I64);
  * p ++ = 13;
  * p ++ = iw_make_o___(OP_CONST_I64);
  * p ++ = (u64) -1;
  * p ++ = iw_make_obb_(OP_PRIM_I64_ADD, 0, 1);
  * p ++ = iw_make_ob__(OP_SHOW_I64, 2);
  * p ++ = iw_make_o___(OP_EXIT);

  disassemble(code, p);

  enum ThreadResult r = interpret(&code[0]);

  switch (r) {
    case THREAD_RESULT_OK:
      printf("ok\n");
      break;
    case THREAD_RESULT_ABORT:
      printf("aborting\n");
      break;
  }

  return 0;
}
