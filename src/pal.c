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
  byte *,
  byte *,
  byte *,
  struct OpTable *
);

struct OpTable {
  OpImpl dispatch[OP_COUNT];
};

STATIC_INLINE u64 var_i64(byte ** ip, byte * sp) {
  return get_i64(sp + pop_i16(ip));
}

STATIC_INLINE enum ThreadResult dispatch(
    byte * ip,
    byte * sp,
    byte * vp,
    struct OpTable * tp
) {
  u16 opcode = pop_i16(&ip);
  TAIL return tp->dispatch[opcode](ip, sp, vp, tp);
}

enum ThreadResult op_abort(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  return THREAD_RESULT_ABORT;
}

/*
enum ThreadResult op_goto(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  byte * a = ip - 1;
  u8 n = pop_i8(&ip);
  i16 k = pop_i16(&ip);
  byte * x = 
  TAIL return dispatch(ip, sp, vp, tp);
}
*/

enum ThreadResult op_exit(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  return THREAD_RESULT_OK;
}

enum ThreadResult op_nop(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_show_i64(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  u64 x = get_i64(sp + pop_i16(&ip));
  printf("%" PRId64 "\n", x);
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_const_i64(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  put_i64(&vp, pop_i64(&ip));
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_prim_i64_add(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  u64 x = var_i64(&ip, sp);
  u64 y = var_i64(&ip, sp);
  u64 z = x + y;
  put_i64(&vp, z);
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_prim_i64_is_eq(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  u64 x = var_i64(&ip, sp);
  u64 y = var_i64(&ip, sp);
  bool z = x == y;
  put_i8(&vp, z);
  TAIL return dispatch(ip, sp, vp, tp);
}

static struct OpTable OP_TABLE = {
  .dispatch = {
    [OP_ABORT] = op_abort,
    [OP_EXIT] = op_exit,
    [OP_NOP] = op_nop,
    [OP_SHOW_I64] = op_show_i64,
    [OP_CONST_I64] = op_const_i64,
    [OP_PRIM_I64_ADD] = op_prim_i64_add,
    [OP_PRIM_I64_IS_EQ] = op_prim_i64_is_eq,
  },
};

enum ThreadResult interpret(byte * ip) {
  byte stack[1024];

  return dispatch(ip, &stack[0], &stack[0], &OP_TABLE);
}

int main(int argc, char ** argv) {
  byte code[1024] = { 0 };

  byte * p = &code[0];

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

  /*
  for (byte * q = &code[0]; q != p; ++ q) {
    printf("%#x\n", (int) * q);
  }
  */

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
