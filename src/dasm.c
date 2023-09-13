static void dump(U64 ic) {
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

  printf("================ DISASSEMBLY ================\n");

  while (p != stop) {
    U64 ic = PEEK_LE(U64, p ++);
    dump(PEEK_LE(U64, q ++));
    printf(" ");

    switch (H0(ic)) {
      case OP_ENTER:
        v = 0;
        printf("enter (");
        n = B2(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          if (i != 0) { printf(", "); }
          printf("%%%d %s", v ++, ty_name(HI(ic, k)));
        }
        printf("):\n");
        break;
      case OP_IF:
        printf("  if %%%d then jump %+d else jump %+d\n", H1(ic), (S16) H2(ic), (S16) H3(ic));
        break;
      case OP_JUMP:
        printf("  jump %+d (", (S16) H2(ic));
        n = H1(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          if (i != 0) { printf(", "); }
          printf("%%%d", HI(ic, k));
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
          printf("%%%d %s", v ++, ty_name(HI(ic, k)));
        }
        printf("):\n");
        break;
      case OP_RET:
        printf("  ret (");
        n = B2(ic);
        for (i = 0; i < n; ++ i) {
          k = i & 3;
          if (k == 0) { ic = PEEK_LE(U64, p ++); }
          if (i != 0) { printf(", "); }
          printf("%%%d", HI(ic, k));
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

    while (p != q) { dump(PEEK_LE(U64, q ++)); printf("\n"); }
  }

  printf("=============================================\n");
}
