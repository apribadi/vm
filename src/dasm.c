static char * op_name(U16 x) {
  switch (x) {
      case OP_ABORT: return "OP_ABORT";
      case OP_IF: return "OP_IF";
      case OP_JUMP: return "OP_JUMP";
      case OP_LABEL: return "OP_LABEL";
      case OP_SHOW_I64: return "OP_SHOW_I64";
      case OP_CONST_I64: return "OP_CONST_I64";
      case OP_I64_ADD: return "OP_I64_ADD";
      case OP_I64_IS_EQ: return "OP_I64_IS_EQ";
      default: return "???";
  }
}
static void disassemble(L64 * ip, L64 * stop) {
  printf("================ DISASSEMBLY ================\n");
  int v = 0;

  while (ip != stop) {
    U64 ic = PEEK_LE(U64, ip ++);
    printf("0x%016" PRIx64 " %s", ic, op_name(H0(ic)));

    switch (H0(ic)) {
      case OP_IF:
        printf(" x%d ? jump %+d : jump %+d\n", H1(ic), (S16) H2(ic), (S16) H3(ic));
        break;
      case OP_JUMP: {
        printf(" jump %+d ( ??? )\n", (S16) H2(ic));
        for (int n = (H1(ic) + 3) / 4; n; -- n) {
          ic = PEEK_LE(U64, ip ++);
          printf("0x%016" PRIx64 "\n", ic);
        }
        break;
      }
      case OP_LABEL: {
        printf(" -> (");
        for (int n = H1(ic); n; -- n) {
          printf(" x%d", v ++);
        }
        printf(" )\n");

        for (int n = (H1(ic) + 3) / 4; n; -- n) {
          ic = PEEK_LE(U64, ip ++);
          printf("0x%016" PRIx64 "\n", ic);
        }
        break;
      }
      case OP_SHOW_I64:
        printf(" ( x%d )\n", H1(ic));
        break;
      case OP_CONST_I64:
        ic = PEEK_LE(U64, ip ++);
        printf(" %" PRIi64 " -> ( x%d )\n", ic, v ++);
        printf("0x%016" PRIx64 "\n", ic);
        break;
      case OP_I64_ADD:
        printf(" ( x%d x%d ) -> ( x%d )\n", H1(ic), H2(ic), v ++);
        break;
      case OP_I64_IS_EQ:
        printf(" ( x%d x%d ) -> ( x%d )\n", H1(ic), H2(ic), v ++);
        break;
      default:
        printf("\n");
        break;
    }
  }
  printf("=============================================\n");
}
