static void hexdump(U64 ic) {
  printf(
      "%04" PRIx16 " %04" PRIx16 " %04" PRIx16 " %04" PRIx16,
      H0(ic),
      H1(ic),
      H2(ic),
      H3(ic)
    );
}

static char * ty_name(TyCode ty) {
  switch(ty) {
    case TY_BOOL: return "Bool";
    case TY_F32: return "F32";
    case TY_F64: return "F64";
    case TY_FUNREF: return "Funref";
    case TY_I32: return "I32";
    case TY_I5: return "I5";
    case TY_I6: return "I6";
    case TY_I64: return "I64";
    case TY_I8: return "I8";
    case TY_V256: return "V256";
  }
  return "???";
}

static void disassemble(L64 * start, L64 * stop) {
  L64 * p = start;
  L64 * q = start;
  int v = 0;
  int n;
  int i;
  int k;

  while (p != stop) {
    U64 ic = PEEK_LE(U64, p ++);
    hexdump(PEEK_LE(U64, q ++));
    printf(" ");

    switch (H0(ic)) {
      case OP_CALL:
        printf("  call %+d (", W1(ic));
        n = H1(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          if (i != 0) { printf(", "); }
          printf("%%%d", H_(ic, k));
        }
        printf(")\n");
        break;
      case OP_CONTINUE:
        printf("  continue");
        n = H1(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          printf(" %+d", H_(ic, k));
        }
        printf("\n");
        break;
      case OP_ENTER:
        v = 0;
        printf("enter (");
        n = H1(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          if (i != 0) { printf(", "); }
          printf("%%%d %s", v ++, ty_name(H_(ic, k)));
        }
        printf("):\n");
        break;
      case OP_IF:
        printf("  if %%%d then %+d else %+d\n", H1(ic), (S16) H2(ic), (S16) H3(ic));
        break;
      case OP_GOTO:
        printf("  goto %+d (", (S16) H2(ic));
        n = H1(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          if (i != 0) { printf(", "); }
          printf("%%%d", H_(ic, k));
        }
        printf(")\n");
        break;
      case OP_LABEL:
        printf("label (");
        n = H1(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          if (i != 0) { printf(", "); }
          printf("%%%d %s", v ++, ty_name(H_(ic, k)));
        }
        printf("):\n");
        break;
      case OP_RETURN:
        printf("  return #%d (", H3(ic));
        n = H1(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          if (i != 0) { printf(", "); }
          printf("%%%d", H_(ic, k));
        }
        printf(")\n");
        break;
      case OP_SHOW_I64:
        printf("  i64.show %%%d\n", H1(ic));
        break;
      case OP_CONST_I64:
        ic = PEEK_LE(U64, p ++);
        printf("  %%%d = i64.const %" PRIi64 "\n", v ++, ic);
        break;
      case OP_I64_ADD:
        printf("  %%%d <- i64.add %%%d %%%d\n", v ++, H1(ic), H2(ic));
        break;
      case OP_I64_IS_EQ:
        printf("  %%%d <- i64.is_eq %%%d %%%d\n", v ++, H1(ic), H2(ic));
        break;
      default:
        printf("  ???\n");
        break;
    }

    while (p != q) {
      hexdump(PEEK_LE(U64, q ++));
      printf("\n");
    }
  }
}
