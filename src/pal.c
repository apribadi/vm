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

typedef enum ThreadResult (*OpImpl)(
  u64 *,
  u64 *,
  u64 *,
  struct OpTable *,
  u64
);

struct OpTable {
  OpImpl dispatch[OP_COUNT];
};

STATIC_INLINE enum ThreadResult dispatch(
    u64 * ip,
    u64 * sp,
    u64 * vp,
    struct OpTable * tp,
    u64 wo
) {
  TAIL return tp->dispatch[wo_h0(wo)](ip, sp, vp, tp, wo);
}

STATIC_INLINE u64 var_i64(u64 * sp, u16 ix) {
  return sp[ix];
}

STATIC_INLINE u32 var_i32(u64 * sp, u16 ix) {
  return var_i64(sp, ix);
}

STATIC_INLINE f32 var_f32(u64 * sp, u16 ix) {
  return i32_to_f32(var_i32(sp, ix));
}

STATIC_INLINE f64 var_f64(u64 * sp, u16 ix) {
  return i64_to_f64(var_i64(sp, ix));
}

STATIC_INLINE void out_i64(u64 * vp, u64 x) {
  * vp = x;
}

STATIC_INLINE void out_i32(u64 * vp, u32 x) {
  out_i64(vp, x);
}

STATIC_INLINE void out_f32(u64 * vp, f32 x) {
  out_i32(vp, f32_to_i32(x));
}

STATIC_INLINE void out_f64(u64 * vp, f64 x) {
  out_i64(vp, f64_to_i64(x));
}

enum ThreadResult op_abort(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  return THREAD_RESULT_ABORT;
}

enum ThreadResult op_exit(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  return THREAD_RESULT_OK;
}

enum ThreadResult op_nop(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  ip += 1;
  TAIL return dispatch(ip, sp, vp, tp, * ip);
}

enum ThreadResult op_show_i64(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  u64 x = var_i64(sp, wo_h1(wo));
  printf("%" PRId64 "\n", x);
  ip += 1;
  TAIL return dispatch(ip, sp, vp, tp, * ip);
}

enum ThreadResult op_const_i32(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  u32 x = wo_w1(wo);
  out_i32(vp, x);
  ip += 1;
  vp += 1;
  TAIL return dispatch(ip, sp, vp, tp, * ip);
}

enum ThreadResult op_const_i64(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  u64 x = ip[1];
  out_i64(vp, x);
  ip += 2;
  vp += 1;
  TAIL return dispatch(ip, sp, vp, tp, * ip);
}

enum ThreadResult op_prim_i32_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  u32 x = var_i32(sp, wo_h1(wo));
  u32 y = var_i32(sp, wo_h2(wo));
  u32 z = x + y;
  out_i32(vp, z);
  ip += 1;
  vp += 1;
  TAIL return dispatch(ip, sp, vp, tp, * ip);
}

enum ThreadResult op_prim_i64_add(u64 * ip, u64 * sp, u64 * vp, struct OpTable * tp, u64 wo) {
  u64 x = var_i64(sp, wo_h1(wo));
  u64 y = var_i64(sp, wo_h2(wo));
  u64 z = x + y;
  out_i64(vp, z);
  ip += 1;
  vp += 1;
  TAIL return dispatch(ip, sp, vp, tp, * ip);
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
    [OP_PRIM_I32_ADD] = op_prim_i32_add,
    [OP_PRIM_I64_ADD] = op_prim_i64_add,
  },
};

enum ThreadResult interpret(u64 * ip) {
  u64 stack[256];

  return dispatch(ip, &stack[0], &stack[0], &OP_TABLE, * ip);
}

int main(int argc, char ** argv) {
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
