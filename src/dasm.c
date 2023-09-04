static void disassemble(b64 * ip, b64 * stop) {
  printf("================ DISASSEMBLY ================\n");
  while (ip != stop) {
    b64 iw = * ip ++;
    printf("0x%016" PRIx64 " ", get_le_u64(&iw));
    switch (get_le_u16(&iw.h0)) {
      case OP_ABORT:
        printf("OP_ABORT\n");
        break;
      case OP_EXIT:
        printf("OP_EXIT\n");
        break;
      case OP_SHOW_I64:
        printf("OP_SHOW_I64 %" PRIi16 "\n", get_le_u16(&iw.h1));
        break;
      case OP_CONST_I64:
        printf("OP_CONST_I64\n");
        if (ip == stop) break;
        iw = * ip ++;
        printf("0x%016" PRIx64 " %" PRIi64 "\n", get_le_u64(&iw), get_le_u64(&iw));
        break;
      case OP_PRIM_I64_ADD:
        printf("OP_CONST_I64 %" PRIi16 " %" PRIi16 "\n", get_le_u16(&iw.h1), get_le_u16(&iw.h2));
        break;
      default:
        printf("UNKNOWN\n");
        break;
    }
  }
  printf("=============================================\n");
}
