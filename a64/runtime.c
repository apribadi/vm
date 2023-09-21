#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

struct Env {
  void * v_stack;
  void * c_stack;
};

struct Stk {
  unsigned char data[256];
} __attribute__((aligned(16)));

extern unsigned char v_main;

noreturn void v_exec_0(void * ep, void * sp, void * fn);

noreturn void v_kont_0(void * ep, void * sp);

noreturn void c_prim_exit(void * ep, void * sp) {
  (void) ep;
  (void) sp;

  // TODO: free stack

  exit(0);
}

noreturn void c_prim_show(void * ep, void * sp, uint64_t x0) {
  printf("%" PRId64 "\n", x0);

  v_kont_0(ep, sp);
}

noreturn void c_prim_hello(void * ep, void * sp) {
  (void) ep;
  (void) sp;

  printf("Hello!\n");

  v_kont_0(ep, sp);
}

noreturn void init(void) {
  struct Env e;
  struct Stk s;
  void * ep = &e;
  void * lp = &s.data[0];
  void * sp = &s.data[256];
  e.v_stack = lp;
  e.c_stack = nullptr;

  v_exec_0(ep, sp, &v_main);
}

int main(int, char **) {
  init();
}
