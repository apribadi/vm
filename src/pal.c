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
  b64 *,
  b64 *,
  b64 *,
  struct OpTable *,
  b64
);

struct OpTable {
  OpFun dispatch[OP_COUNT];
};

STATIC_INLINE enum ThreadResult dispatch(
    b64 * ip,
    b64 * sp,
    b64 * vp,
    struct OpTable * tp,
    b64
) {
  b64 iw = * ip ++;
  TAIL return tp->dispatch[get_le_u16(&iw.h0)](ip, sp, vp, tp, iw);
}

STATIC_INLINE bool var_bool(b64 * sp, b16 * ix) {
  return get_bool(&sp[get_le_u16(ix)]);
}

STATIC_INLINE u32 var_i32(b64 * sp, b16 * ix) {
  return get_u32(&sp[get_le_u16(ix)]);
}

STATIC_INLINE u64 var_i64(b64 * sp, b16 * ix) {
  return get_u64(&sp[get_le_u16(ix)]);
}

STATIC_INLINE f32 var_f32(b64 * sp, b16 * ix) {
  return get_f32(&sp[get_le_u16(ix)]);
}

STATIC_INLINE f64 var_f64(b64 * sp, b16 * ix) {
  return get_f64(&sp[get_le_u16(ix)]);
}

static enum ThreadResult op_abort(b64 *, b64 *, b64 *, struct OpTable *, b64) {
  return THREAD_RESULT_ABORT;
}

static enum ThreadResult op_branch(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  bool p = var_bool(sp, &iw.h1);
  s16 k = p ? get_le_s16(&iw.h2) : get_le_s16(&iw.h3);

  ip = ip - 1 + k;
  iw = * ip ++;
  vp = sp + get_le_u16(&iw.h2);

  assert(get_le_u16(&iw.h0) == OP_LABEL);
  assert(get_le_u16(&iw.h1) == 0);

  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_exit(b64 *, b64 *, b64 *, struct OpTable *, b64) {
  return THREAD_RESULT_OK;
}

static b64 tmp[256];

static enum ThreadResult op_jump(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  u16 n = get_le_u16(&iw.h1); // num args
  s16 k = get_le_s16(&iw.h2); // jump offset

  b64 * ap = ip; // argument pointer
  b64 * bp = tmp; // ???
  b64 * cp = tmp; // ???

  ip = ip - 1 + k;
  iw = * ip ++;
  vp = sp + get_le_u16(&iw.h2);

  assert(get_le_u16(&iw.h0) == OP_LABEL);
  assert(get_le_u16(&iw.h1) == n);

  b64 x;
  b64 y;
  u16 i = 0;

  for (;;) {
    // TODO: multi-word values

    if (i ++ == n) break;
    x = * ap ++;
    y = * ip ++;
    * bp ++ = sp[get_le_u16(&x.h0)];
    if (i ++ == n) break;
    * bp ++ = sp[get_le_u16(&x.h1)];
    if (i ++ == n) break;
    * bp ++ = sp[get_le_u16(&x.h2)];
    if (i ++ == n) break;
    * bp ++ = sp[get_le_u16(&x.h3)];
    if (i ++ == n) break;
    x = * ap ++;
    * bp ++ = sp[get_le_u16(&x.h0)];
    if (i ++ == n) break;
    * bp ++ = sp[get_le_u16(&x.h1)];
    if (i ++ == n) break;
    * bp ++ = sp[get_le_u16(&x.h2)];
    if (i ++ == n) break;
    * bp ++ = sp[get_le_u16(&x.h3)];

    (void) y;
  }

  while (cp != bp) {
    * vp ++ = * cp ++;
  }

  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_nop(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_show_i64(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  u64 x = var_i64(sp, &iw.h1);
  printf("%" PRIi64 "\n", x);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_const_f32(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  f32 x = get_le_f32(&iw.w1);
  set_f32(vp ++, x);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_const_f64(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  f64 x = get_le_f64(ip ++);
  set_f64(vp ++, x);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_const_i32(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  u32 x = get_le_u32(&iw.w1);
  set_u32(vp ++, x);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_const_i64(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  u64 x = get_le_u64(ip ++);
  set_u64(vp ++, x);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_f32_add(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  f32 x = var_f32(sp, &iw.h1);
  f32 y = var_f32(sp, &iw.h2);
  f32 z = x + y;
  set_f32(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_f32_sqrt(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  f32 x = var_f32(sp, &iw.h1);
  f32 y = sqrtf(x);
  set_f32(vp ++, y);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_f64_add(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  f64 x = var_f64(sp, &iw.h1);
  f64 y = var_f64(sp, &iw.h2);
  f64 z = x + y;
  set_f64(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_f64_sqrt(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  f64 x = var_f64(sp, &iw.h1);
  f64 y = sqrt(x);
  set_f64(vp ++, y);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_i32_add(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  u32 x = var_i32(sp, &iw.h1);
  u32 y = var_i32(sp, &iw.h2);
  u32 z = x + y;
  set_u32(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_i64_add(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  u64 x = var_i64(sp, &iw.h1);
  u64 y = var_i64(sp, &iw.h2);
  u64 z = x + y;
  set_u64(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static enum ThreadResult op_prim_i64_is_eq(b64 * ip, b64 * sp, b64 * vp, struct OpTable * tp, b64 iw) {
  u64 x = var_i64(sp, &iw.h1);
  u64 y = var_i64(sp, &iw.h2);
  bool z = x == y;
  set_bool(vp ++, z);
  TAIL return dispatch(ip, sp, vp, tp, iw);
}

static struct OpTable OP_TABLE = {
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

static enum ThreadResult interpret(b64 * ip) {
  b64 stack[256] = { 0 };

  return dispatch(ip, &stack[0], &stack[0], &OP_TABLE, iw_make_d___(0));
}

int main(int, char **) {
  b64 code[256] = { 0 };

  b64 * p = &code[0];

  (void) p;

  /*

  * p ++ = iw_make_o___(OP_CONST_I64);
  * p ++ = iw_make_d___(13);
  * p ++ = iw_make_o___(OP_CONST_I64);
  * p ++ = iw_make_d___((u64) -1);
  * p ++ = iw_make_ohh_(OP_PRIM_I64_ADD, 0, 1);
  * p ++ = iw_make_oh__(OP_SHOW_I64, 2);
  * p ++ = iw_make_o___(OP_EXIT);

  */

  disassemble(code, p);

  enum ThreadResult r = interpret(&code[0]);

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
