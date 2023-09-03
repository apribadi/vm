static void disassemble(u64 * ip, u64 * stop) {
  printf("================ DISASSEMBLY ================\n");
  while (ip != stop) {
    u64 iw = * ip ++;
    printf("0x%016" PRIx64 " ", iw);
    switch (iw_b0(iw)) {
      case OP_ABORT:
        printf("OP_ABORT\n");
        break;
      case OP_EXIT:
        printf("OP_EXIT\n");
        break;
      case OP_SHOW_I64:
        printf("OP_SHOW_I64 %" PRIi16 "\n", iw_b1(iw));
        break;
      case OP_CONST_I64:
        printf("OP_CONST_I64\n");
        if (ip == stop) break;
        iw = * ip ++;
        printf("0x%016" PRIx64 " %" PRIi64 "\n", iw, iw);
        break;
      case OP_PRIM_I64_ADD:
        printf("OP_CONST_I64 %" PRIi16 " %" PRIi16 "\n", iw_b1(iw), iw_b2(iw));
        break;
      default:
        printf("UNKNOWN\n");
        break;
    }
  }
  printf("=============================================\n");
}
