#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STATIC_INLINE static __attribute__((always_inline))
#define TAIL __attribute__((musttail))
#define OP_PARAMS byte * ip, byte * sp, byte * vp, struct ThreadInfo * tp
#define OP_DISPATCH __attribute__((musttail)) return dispatch(ip, sp, vp, tp)

#include "int.c"
#include "byte.c"
#include "code.c"

enum ThreadResult: u8 {
  THREAD_RESULT_OK,
  THREAD_RESULT_ABORT,
};

struct ThreadInfo;

typedef enum ThreadResult (*OpImpl)(
  byte *,
  byte *,
  byte *,
  struct ThreadInfo *
);

struct ThreadInfo {
  OpImpl dispatch[256];
};

STATIC_INLINE enum ThreadResult dispatch(
    byte * ip,
    byte * sp,
    byte * vp,
    struct ThreadInfo * tp
) {
  u8 opcode = pop_u8(&ip);
  TAIL return tp->dispatch[opcode](ip, sp, vp, tp);
}

enum ThreadResult op_abort(
  byte * ip,
  byte * sp,
  byte * vp,
  struct ThreadInfo * tp
) {
  return THREAD_RESULT_ABORT;
}

enum ThreadResult op_nop(
  byte * ip,
  byte * sp,
  byte * vp,
  struct ThreadInfo * tp
) {
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_const_i64(byte * ip, byte * sp, byte * vp, struct ThreadInfo * tp) {
  put_u64(&vp, pop_u64(&ip));
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_prim_i64_add(byte * ip, byte * sp, byte * vp, struct ThreadInfo * tp) {
  u64 x = get_u64(sp + pop_u16(&ip));
  u64 y = get_u64(sp + pop_u16(&ip));
  u64 z = x + y;
  put_u64(&vp, z);
  TAIL return dispatch(ip, sp, vp, tp);
}

enum ThreadResult op_show_i64(byte * ip, byte * sp, byte * vp, struct ThreadInfo * tp) {
  u64 x = get_u64(sp + pop_u16(&ip));
  printf("%" PRId64 "\n", x);
  TAIL return dispatch(ip, sp, vp, tp);
}

void interpret(byte * ip) {
  byte stack[1024];

  struct ThreadInfo thread_info = {
    .dispatch = {
      [OP_ABORT] = op_abort,
      [OP_CONST_I64] = op_const_i64,
      [OP_NOP] = op_nop,
      [OP_PRIM_I64_ADD] = op_prim_i64_add,
      [OP_SHOW_I64] = op_show_i64,
    },
  };

  dispatch(ip, &stack[0], &stack[0], &thread_info);
}

int main(int argc, char ** argv) {
  (void) argc;
  (void) argv;

  printf("Hello!\n");

  byte code[1024] = { 0 };

  byte * p = &code[0];

  put_u8(&p, OP_CONST_I64);
  put_u64(&p, 13);
  put_u8(&p, OP_CONST_I64);
  put_u64(&p, 3);
  put_u8(&p, OP_PRIM_I64_ADD);
  put_u16(&p, 0);
  put_u16(&p, 8);
  put_u8(&p, OP_SHOW_I64);
  put_u16(&p, 16);
  put_u8(&p, OP_ABORT);

  /*
  for (byte * q = &code[0]; q != p; ++ q) {
    printf("%#x\n", (int) * q);
  }
  */

  interpret(&code[0]);

  return 0;
}

