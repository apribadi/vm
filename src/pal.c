#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prelude.c"
#include "code.c"
#include "vm.c"
#include "dasm.c"

int main(int, char **) {
  L64 code[] = {
    /*   */ ic_make_hhhh(OP_ENTER, 0, 2, 1),
    /* 0 */ ic_make_h___(OP_CONST_I64),
    /*   */ ic_make_d___(10),
    /*   */ ic_make_hhw_(OP_CALL, 1, 8),
    /*   */ ic_make_h___(0),
    /*   */ ic_make_hhh_(OP_CONTINUE, 1, 2),
    /*   */ ic_make_h___(2),
    /* 1 */ ic_make_hhh_(OP_LABEL, 1, 1),
    /*   */ ic_make_h___(TY_I64),
    /*   */ ic_make_hh__(OP_SHOW_I64, 1),
    /*   */ ic_make_hhhh(OP_RETURN, 0, 2, 0),
    /* 0 */ ic_make_hhhh(OP_ENTER, 1, 10, 1),
    /*   */ ic_make_h___(TY_I64),
    /* 1 */ ic_make_h___(OP_CONST_I64),
    /*   */ ic_make_d___(0),
    /* 2 */ ic_make_hhh_(OP_I64_IS_EQ, 0, 1),
    /*   */ ic_make_hhhh(OP_IF, 2, 1, 4),
    /*   */ ic_make_hhh_(OP_LABEL, 0, 3),
    /*   */ ic_make_hhhh(OP_RETURN, 1, 10, 0),
    /*   */ ic_make_h___(1),
    /*   */ ic_make_hhh_(OP_LABEL, 0, 3),
    /* 3 */ ic_make_h___(OP_CONST_I64),
    /*   */ ic_make_d___(1),
    /*   */ ic_make_hhh_(OP_GOTO, 3, 2),
    /*   */ ic_make_hhh_(3, 1, 3),
    /* 4 */ ic_make_hhh_(OP_LABEL, 3, 4),
    /*   */ ic_make_hhh_(TY_I64, TY_I64, TY_I64),
    /* 7 */ ic_make_hhh_(OP_I64_IS_EQ, 4, 0),
    /*   */ ic_make_hhhh(OP_IF, 7, 6, 1),
    /*   */ ic_make_hhh_(OP_LABEL, 0, 8),
    /* 8 */ ic_make_hhh_(OP_I64_ADD, 5, 6),
    /* 9 */ ic_make_hhh_(OP_I64_ADD, 4, 3),
    /*   */ ic_make_hhh_(OP_GOTO, 3, (U16) -7),
    /*   */ ic_make_hhh_(9, 6, 8),
    /*   */ ic_make_hhh_(OP_LABEL, 0, 10),
    /*   */ ic_make_hhhh(OP_RETURN, 1, 10, 0),
    /*   */ ic_make_h___(6),
  };

  disassemble(code, code + sizeof(code) / sizeof(L64));

  ExitCode r = interpret(&code[0]);

  switch (r) {
    case EXIT_CODE_OK:
      printf("ok\n");
      break;
    case EXIT_CODE_PANIC:
      printf("panic!\n");
      break;
  }

  return 0;
}
