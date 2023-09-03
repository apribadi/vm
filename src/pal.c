#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "prelude.c"
#include "code.c"

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
    u64 wo
) {
  wo = * ip ++;
  TAIL return tp->dispatch[wo_h0(wo)](ip, sp, vp, tp, wo);
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

static enum ThreadResult op_abort(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  (void) ip;
  (void) sp;
  (void) vp;
  (void) tp;
  (void) wo;

  return THREAD_RESULT_ABORT;
}

static enum ThreadResult op_exit(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  (void) ip;
  (void) sp;
  (void) vp;
  (void) tp;
  (void) wo;

  return THREAD_RESULT_OK;
}

static enum ThreadResult op_nop(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  TAIL return dispatch(ip, sp, vp, tp, wo);
}

static enum ThreadResult op_show_i64(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  u64 x = var_i64(sp, wo_h1(wo));
  printf("%" PRId64 "\n", x);
  TAIL return dispatch(ip, sp, vp, tp, wo);
}

static enum ThreadResult op_const_i32(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  * vp ++ = wo_w1(wo);
  TAIL return dispatch(ip, sp, vp, tp, wo);
}

static enum ThreadResult op_const_i64(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  * vp ++ = * ip ++;
  TAIL return dispatch(ip, sp, vp, tp, wo);
}

static enum ThreadResult op_prim_f32_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  f32 x = var_f32(sp, wo_h1(wo));
  f32 y = var_f32(sp, wo_h2(wo));
  f32 z = x + y;
  * vp ++ = bitcast_f32_to_u32(z);
  TAIL return dispatch(ip, sp, vp, tp, wo);
}

static enum ThreadResult op_prim_f64_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  f64 x = var_f64(sp, wo_h1(wo));
  f64 y = var_f64(sp, wo_h2(wo));
  f64 z = x + y;
  * vp ++ = bitcast_f64_to_u64(z);
  TAIL return dispatch(ip, sp, vp, tp, wo);
}

static enum ThreadResult op_prim_i32_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  u32 x = var_i32(sp, wo_h1(wo));
  u32 y = var_i32(sp, wo_h2(wo));
  u32 z = x + y;
  * vp ++ = z;
  TAIL return dispatch(ip, sp, vp, tp, wo);
}

static enum ThreadResult op_prim_i64_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  u64 x = var_i64(sp, wo_h1(wo));
  u64 y = var_i64(sp, wo_h2(wo));
  u64 z = x + y;
  * vp ++ = z;
  TAIL return dispatch(ip, sp, vp, tp, wo);
}

static struct OpTable OP_TABLE = {
  .dispatch = {
    [OP_ABORT] = op_abort,
    [OP_EXIT] = op_exit,
    [OP_NOP] = op_nop,
    [OP_SHOW_I64] = op_show_i64,
    [OP_CONST_F32] = op_const_i32, // same as op_const_i32
    [OP_CONST_F64] = op_const_i64, // same as op_const_i64
    [OP_CONST_I32] = op_const_i32,
    [OP_CONST_I64] = op_const_i64,
    [OP_PRIM_F32_ADD] = op_prim_f32_add,
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

  * p ++ = wo_make_o___(OP_CONST_I64);
  * p ++ = 13;
  * p ++ = wo_make_o___(OP_CONST_I64);
  * p ++ = 3;
  * p ++ = wo_make_ohh_(OP_PRIM_I64_ADD, 0, 1);
  * p ++ = wo_make_oh__(OP_SHOW_I64, 2);
  * p ++ = wo_make_o___(OP_EXIT);

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
