#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

struct Env {
  void * c_stack;
};

struct Stk {
  unsigned char data[256];
} __attribute__((aligned(16)));

extern unsigned char v_main;

noreturn void v_exec_0(void * ep, void * lp, void * sp, void * fn);

noreturn void v_kont_0(void * ep, void * lp, void * sp);

noreturn void c_prim_exit(void * ep, void * lp, void * sp) {
  (void) ep;
  (void) lp;
  (void) sp;

  // TODO: free stack

  exit(0);
}

noreturn void c_prim_show(void * ep, void * lp, void * sp, uint64_t x0) {
  printf("%" PRId64 "\n", x0);

  v_kont_0(ep, lp, sp);
}

noreturn void c_prim_hello(void * ep, void * lp, void * sp) {
  (void) ep;
  (void) lp;
  (void) sp;

  printf("Hello!\n");

  v_kont_0(ep, lp, sp);
}

noreturn void init(void) {
  struct Env e;
  struct Stk s;
  void * ep = &e;
  void * lp = &s.data[0];
  void * sp = &s.data[256];

  v_exec_0(ep, lp, sp, &v_main);
}

int main(int, char **) {
  init();
}
