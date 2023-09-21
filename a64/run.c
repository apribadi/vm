#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

typedef int noret;

struct Env {
  void * c_sp;
};

struct Stk {
  unsigned char data[256];
} __attribute__((aligned(16)));

void foo(void);

noreturn void v_exec_0(void * ep, void * lp, void * sp, void * fn);

noreturn void v_kont_0(void * ep, void * lp, void * sp, void * rt);

noreturn void c_prim_abort(void * ep, void * lp, void * sp, void * rt) {
  (void) ep;
  (void) lp;
  (void) sp;
  (void) rt;

  abort();
}

noreturn void c_prim_exit(void * ep, void * lp, void * sp, void * rt) {
  (void) ep;
  (void) lp;
  (void) sp;
  (void) rt;

  // TODO: free stack

  exit(0);
}

noreturn void c_prim_show(void * ep, void * lp, uint64_t x0, void * sp, void * rt) {
  printf("%" PRId64 "\n", x0);
  v_kont_0(ep, lp, sp, rt);
}


noreturn void init(void) {
  struct Env e;
  struct Stk s;
  void * ep = &e;
  void * lp = &s.data[0];
  void * sp = &s.data[256];

  v_exec_0(ep, lp, sp, (void *) &foo);
}

int main(int, char **) {
  init();
}
