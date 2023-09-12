static void disassemble(L64 * ip, L64 * stop) {
  printf("================ DISASSEMBLY ================\n");
  int v = 0;

  while (ip != stop) {
    U64 ic = PEEK_LE(U64, ip ++);
    printf("0x%016" PRIx64 " ", ic);

    switch (H0(ic)) {
      case OP_ENTER:
        printf("enter (");
        for (int n = B2(ic); n; -- n) {
          printf(" %%%d", v ++);
        }
        printf(" ):\n");
        for (int n = (B2(ic) + 3) / 4; n; -- n) {
          ic = PEEK_LE(U64, ip ++);
          printf("0x%016" PRIx64 "\n", ic);
        }
        break;
      case OP_IF:
        printf("  if %%%d then jump %+d else jump %+d\n", H1(ic), (S16) H2(ic), (S16) H3(ic));
        break;
      case OP_JUMP:
        printf("  jump %+d ( ??? )\n", (S16) H2(ic));
        for (int n = (H1(ic) + 3) / 4; n; -- n) {
          ic = PEEK_LE(U64, ip ++);
          printf("0x%016" PRIx64 "\n", ic);
        }
        break;
      case OP_LABEL:
        printf("label (");
        for (int n = H1(ic); n; -- n) {
          printf(" %%%d", v ++);
        }
        printf(" ):\n");
        for (int n = (H1(ic) + 3) / 4; n; -- n) {
          ic = PEEK_LE(U64, ip ++);
          printf("0x%016" PRIx64 "\n", ic);
        }
        break;
      case OP_RET:
        printf("  ret (");
        printf(" )\n");
        break;
      case OP_SHOW_I64:
        printf("  i64.show %%%d\n", H1(ic));
        break;
      case OP_CONST_I64:
        ic = PEEK_LE(U64, ip ++);
        printf("  %%%d = i64.const %" PRIi64 "\n", v ++, ic);
        printf("0x%016" PRIx64 "\n", ic);
        break;
      case OP_I64_ADD:
        printf("  %%%d <- i64+ %%%d %%%d\n", v ++, H1(ic), H2(ic));
        break;
      case OP_I64_IS_EQ:
        printf("  %%%d <- i64== %%%d %%%d\n", v ++, H1(ic), H2(ic));
        break;
      default:
        printf("  ???\n");
        break;
    }
  }
  printf("=============================================\n");
}
