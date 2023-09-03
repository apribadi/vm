#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STATIC_INLINE static inline __attribute__((always_inline))
#define TAIL __attribute__((musttail))

#include "base.c"
#include "byte.c"
#include "code.c"

enum ThreadResult: u8 {
  THREAD_RESULT_OK,
  THREAD_RESULT_ABORT,
};

struct OpTable;

typedef enum ThreadResult (*OpImpl)(
  u64 *,
  byte *,
  byte *,
  struct OpTable *,
  enum Wo
);

struct OpTable {
  OpImpl dispatch[OP_COUNT];
};

STATIC_INLINE enum ThreadResult dispatch(
    u64 * ip,
    byte * sp,
    byte * vp,
    struct OpTable * tp,
    enum Wo wo
) {
  TAIL return tp->dispatch[wo_op(wo)](ip, sp, vp, tp, wo);
}

enum ThreadResult op_abort(u64 * ip, byte * sp, byte * vp, struct OpTable * tp, enum Wo wo) {
  return THREAD_RESULT_ABORT;
}

enum ThreadResult op_exit(u64 * ip, byte * sp, byte * vp, struct OpTable * tp, enum Wo wo) {
  return THREAD_RESULT_OK;
}

enum ThreadResult op_nop(u64 * ip, byte * sp, byte * vp, struct OpTable * tp, enum Wo wo) {
  ip ++;
  TAIL return dispatch(ip, sp, vp, tp, *ip);
}

enum ThreadResult op_show_i64(u64 * ip, byte * sp, byte * vp, struct OpTable * tp, enum Wo wo) {
  ip ++;
  u64 x = get_i64(sp + wo_h1(wo));
  printf("%" PRId64 "\n", x);
  TAIL return dispatch(ip, sp, vp, tp, *ip);
}

enum ThreadResult op_const_i64(u64 * ip, byte * sp, byte * vp, struct OpTable * tp, enum Wo wo) {
  ip ++;
  u64 x = * ip ++;
  put_i64(&vp, x);
  TAIL return dispatch(ip, sp, vp, tp, *ip);
}

enum ThreadResult op_prim_i64_add(u64 * ip, byte * sp, byte * vp, struct OpTable * tp, enum Wo wo) {
  ip ++;
  u64 x = get_i64(sp + wo_h1(wo));
  u64 y = get_i64(sp + wo_h2(wo));
  u64 z = x + y;
  put_i64(&vp, z);
  TAIL return dispatch(ip, sp, vp, tp, *ip);
}

static struct OpTable OP_TABLE = {
  .dispatch = {
    [OP_ABORT] = op_abort,
    [OP_EXIT] = op_exit,
    [OP_NOP] = op_nop,
    [OP_SHOW_I64] = op_show_i64,
    [OP_CONST_I64] = op_const_i64,
    [OP_PRIM_I64_ADD] = op_prim_i64_add,
  },
};

enum ThreadResult interpret(u64 * ip) {
  byte stack[1024];

  return dispatch(ip, &stack[0], &stack[0], &OP_TABLE, *ip);
}

int main(int argc, char ** argv) {
  u64 code[1024] = { 0 };

  u64 * p = &code[0];

  /*
  put_i16(&p, OP_CONST_I64);
  put_i64(&p, 13);
  put_i16(&p, OP_CONST_I64);
  put_i64(&p, 3);
  put_i16(&p, OP_PRIM_I64_ADD);
  put_i16(&p, 0);
  put_i16(&p, 8);
  put_i16(&p, OP_SHOW_I64);
  put_i16(&p, 16);
  put_i16(&p, OP_EXIT);
  */

  * p ++ = wo_make_o___(OP_CONST_I64);
  * p ++ = 13;
  * p ++ = wo_make_o___(OP_CONST_I64);
  * p ++ = 3;
  * p ++ = wo_make_ohh_(OP_PRIM_I64_ADD, 0, 8);
  * p ++ = wo_make_oh__(OP_SHOW_I64, 16);
  * p ++ = wo_make_o___(OP_EXIT);

  enum ThreadResult r = interpret(&code[0]);

  switch (r) {
    case THREAD_RESULT_OK:
      printf("exit ok\n");
      break;
    case THREAD_RESULT_ABORT:
      printf("aborting ...\n");
      break;
  }

  return 0;
}
