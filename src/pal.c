#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STATIC_INLINE static inline __attribute__((always_inline))
#define TAIL __attribute__((musttail))

#include "int.c"
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
  OpImpl dispatch[256];
};

STATIC_INLINE enum ThreadResult dispatch(
    byte * ip,
    byte * sp,
    byte * vp,
    struct OpTable * tp
) {
  u8 opcode = pop_u8(&ip);
  TAIL return tp->dispatch[opcode](ip, sp, vp, tp);
}

enum ThreadResult op_abort(
  byte * ip,
  byte * sp,
  byte * vp,
  struct OpTable * tp
) {
  return THREAD_RESULT_ABORT;
}

enum ThreadResult op_nop(
  byte * ip,
  byte * sp,
  byte * vp,
  struct OpTable * tp
) {
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_show_i64(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  u64 x = get_u64(sp + pop_u16(&ip));
  printf("%" PRId64 "\n", x);
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_y_const_i64(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  put_u64(&vp, pop_u64(&ip));
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_z_i64_add(byte * ip, byte * sp, byte * vp, struct OpTable * tp) {
  u64 x = get_u64(sp + pop_u16(&ip));
  u64 y = get_u64(sp + pop_u16(&ip));
  u64 z = x + y;
  put_u64(&vp, z);
  TAIL return dispatch(ip, sp, vp, tp);
}

static struct OpTable OP_TABLE = {
  .dispatch = {
    [OP_ABORT] = op_abort,
    [OP_NOP] = op_nop,
    [OP_SHOW_I64] = op_show_i64,
    [OP_Y_CONST_I64] = op_y_const_i64,
    [OP_Z_I64_ADD] = op_z_i64_add,
  },
};

void interpret(byte * ip) {
  byte stack[1024];

  dispatch(ip, &stack[0], &stack[0], &OP_TABLE);
}

STATIC_INLINE void emit_op(byte ** p, enum Op op) {
  put_u8(p, op);
}

int main(int argc, char ** argv) {
  (void) argc;
  (void) argv;

  printf("Hello!\n");

  byte code[1024] = { 0 };

  byte * p = &code[0];

  emit_op(&p, OP_Y_CONST_I64);
  put_u64(&p, 13);
  emit_op(&p, OP_Y_CONST_I64);
  put_u64(&p, 3);
  emit_op(&p, OP_Z_I64_ADD);
  put_u16(&p, 0);
  put_u16(&p, 8);
  emit_op(&p, OP_SHOW_I64);
  put_u16(&p, 16);
  emit_op(&p, OP_ABORT);

  /*
  for (byte * q = &code[0]; q != p; ++ q) {
    printf("%#x\n", (int) * q);
  }
  */

  interpret(&code[0]);

  return 0;
}
