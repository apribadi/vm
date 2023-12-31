typedef enum ExitCode : U32 {
  EXIT_CODE_OK,
  EXIT_CODE_PANIC,
} ExitCode;

struct Env;

typedef ExitCode (* OpHandler)(
  struct Env *,
  L64 *,
  U64 *,
  U64 *,
  U64 *,
  U64
);

typedef struct Env {
  U64 * bp;
  U64 * lp;
  OpHandler dispatch[OP_COUNT];
} Env;

static inline ExitCode vm_dispatch(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp) {
  U64 ic = PEEK_LE(U64, ip ++);
  return ep->dispatch[H0(ic)](ep, ip, fp, vp, sp, ic);
}

__attribute__((noinline))
static ExitCode vm_grow_stack(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp) {
  (void) ep;
  (void) ip;
  (void) fp;
  (void) vp;
  (void) sp;

  return EXIT_CODE_PANIC;
}

static ExitCode vm_abort(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  (void) ep;
  (void) ip;
  (void) fp;
  (void) vp;
  (void) sp;

  return W1(ic);
}

static ExitCode vm_call(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | OP_CALL         | # args          |              dst disp             |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // | arg 4           | arg 5           |                 |                 |
  // | OP_CONTINUE     | # konts         | frame size      |                 |
  // | kont 0          | kont 1          |                 |                 |
  // |                 |                 |                 |                 |
  // | OP_CALL_IND...  | # args          | funref          |                 |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // | arg 4           | arg 5           |                 |                 |
  // | OP_CONTINUE     | # konts         | frame size      |                 |
  // | kont 0          | kont 1          |                 |                 |
  // |                 |                 |                 |                 |
  // | OP_LABEL        | # args          | next var        |                 |
  // | type 0          | type 1          | type 2          | type 3          |
  // |                 |                 |                 |                 |
  // | OP_ENTER        | # args          | frame size      | # konts         |
  // | type 0          | type 1          | type 2          | type 3          |
  // |                 |                 |                 |                 |
  // | OP_RETURN       | # args          | frame size      | kont index      |
  // | retval 0        | retval 1        | retval 2        | retval 3        |

  //         sp1 --> +-----+
  //                 | ooo |
  //                 +-----+
  //                 |     |
  //                 |     |
  // sp0 --> fp1 --> +-----+
  //                 | ret |
  //                 +-----+
  //                 |     |
  //                 |     |
  // fp0 ----------> +-----+

  L64 * ip0 = ip;
  U64 * fp0 = fp;
  U64 * vp0 = vp;
  U64 * sp0 = sp;

  U16 an = H1(ic); // # args

  // enter function

  switch (H0(ic)) {
    case OP_CALL:
      ip = ip - 1 + (S32) W1(ic);
      break;
    case OP_CALL_INDIRECT:
      ip = PEEK(L64 *, fp + H2(ic));
      break;
    default:
      __builtin_unreachable();
  }

  ic = PEEK_LE(U64, ip ++);

  ASSERT(H0(ic) == OP_ENTER);
  ASSERT(H1(ic) == an);
  ASSERT(H3(ic) == H1(PEEK_LE(U64, ip0 + ((size_t) an + 3) / 4)));

  // stack overflow check

  U32 fz = (U32) H2(ic) + 1;

  if (! (fz <= ep->lp - sp)) {
    return vm_grow_stack(ep, ip0, fp0, vp0, sp0);
  }

  // make new stack frame

  fp = sp;
  vp = fp;
  sp = fp + fz;

  // copy args

  if (an) {
    while (true) {
      U64 ic0 = PEEK_LE(U64, ip0 ++);
      ip ++;

      * vp ++ = fp0[H0(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H1(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H2(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H3(ic0)]; if (! -- an) break;
    }
  }

  // store return address

  POKE(L64 *, fp - 1, ip0);

  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_goto(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | OP_GOTO         | # args          | dst disp        |                 |
  // | arg 0           | arg 1           | arg 2           | arg 3           |
  // |                 |                 |                 |                 |
  // | OP_LABEL        | # args          | next var        |                 |
  // | type 0          | type 1          | type 2          | type 3          |

  L64 * ip0 = ip;

  U16 an = H1(ic);
  S16 di = (S16) H2(ic);

  ip = ip - 1 + di;
  ic = PEEK_LE(U64, ip ++);
  vp = fp + H2(ic);

  ASSERT(H0(ic) == OP_LABEL);
  ASSERT(H1(ic) == an);

  if (an) {
    // TODO: stack overflow check

    U64 * ap = sp;
    U64 * bp = sp;

    // copy args into scratch space

    while (true) {
      U64 ic0 = PEEK_LE(U64, ip0 ++);
      ip ++;

      * ap ++ = fp[H0(ic0)]; if (! -- an) break;
      * ap ++ = fp[H1(ic0)]; if (! -- an) break;
      * ap ++ = fp[H2(ic0)]; if (! -- an) break;
      * ap ++ = fp[H3(ic0)]; if (! -- an) break;
    }

    // copy args from scratch space

    while (bp != ap) {
      * vp ++ = * bp ++;
    }
  }

  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_nop(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  (void) ic;

  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_if(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | OP_IF           | pred            | dst disp 0      | dst disp 0      |
  // |                 |                 |                 |                 |
  // | OP_LABEL        | # args (= 0)    | next var        |                 |

  Bool p = PEEK(Bool, fp + H1(ic));
  S16 a = (S16) H2(ic);
  S16 b = (S16) H3(ic);
  S16 di = p ? a : b;

  ip = ip - 1 + di;
  ic = PEEK_LE(U64, ip ++);
  vp = fp + H2(ic);

  ASSERT(H0(ic) == OP_LABEL);
  ASSERT(H1(ic) == 0);

  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_return(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  // | H0              | H1              | H2              | H3              |
  // |                 |                 |                 |                 |
  // | OP_RETURN       | # retvals       | frame size      | kont index      |
  // | retval 0        | retval 1        | retval 2        | retval 3        |
  //
  // | OP_CONTINUE     | # konts         | frame size      |                 |
  // | kont 0          | kont 1          |                 |                 |
  // |                 |                 |                 |                 |
  // | OP_LABEL        | # args          | next var        |                 |
  // | type 0          | type 1          | type 2          | type 3          |

  // sp0 ----------> +-----+
  //                 | ooo |
  //                 +-----+
  //                 |     |
  //                 |     |
  // fp0 --> sp1 --> +-----+
  //                 | ret |
  //                 +-----+
  //                 |     |
  //                 |     |
  //         fp1 --> +-----+

  L64 * ip0 = ip;
  U64 * fp0 = fp;

  U16 an = H1(ic);
  U16 ki = H3(ic);

  ip = PEEK(L64 *, fp - 1);
  ic = PEEK_LE(U64, ip ++);
  sp = fp;
  fp = fp - 1 - H2(ic);

  ASSERT(H0(ic) == OP_CONTINUE);
  ASSERT(ki < H1(ic));

  S16 di = (S16) PEEK_LE(U16, (L16 *) ip + ki);

  ip = ip - 1 + di;
  ic = PEEK_LE(U64, ip ++);
  vp = fp + H2(ic);

  ASSERT(H0(ic) == OP_LABEL);
  ASSERT(H1(ic) == an);

  if (an) {
    while (true) {
      U64 ic0 = PEEK_LE(U64, ip0 ++);
      ip ++;

      * vp ++ = fp0[H0(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H1(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H2(ic0)]; if (! -- an) break;
      * vp ++ = fp0[H3(ic0)]; if (! -- an) break;
   }
  }

  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_select(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  Bool p = PEEK(Bool, fp + H1(ic));
  U16 i = p ? H2(ic) : H3(ic);
  * vp ++ = fp[i];
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_show_i64(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  printf("%" PRIi64 "\n", x);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_const_f32(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U32 x = W1(ic);
  F32 y = PEEK(F32, &x);
  POKE(F32, vp ++, y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_const_f64(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  (void) ic;

  U64 x = PEEK_LE(U64, ip ++);
  F64 y = PEEK(F64, &x);
  POKE(F64, vp ++, y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_const_i32(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U32 x = W1(ic);
  POKE(U32, vp ++, x);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_const_i64(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  (void) ic;

  U64 x = PEEK_LE(U64, ip ++);
  POKE(U64, vp ++, x);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_f32_add(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  F32 x = PEEK(F32, fp + H1(ic));
  F32 y = PEEK(F32, fp + H2(ic));
  POKE(F32, vp ++, x + y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_f32_sqrt(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  F32 x = PEEK(F32, fp + H1(ic));
  POKE(F32, vp ++, sqrtf(x));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_f64_add(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  F64 x = PEEK(F64, fp + H1(ic));
  F64 y = PEEK(F64, fp + H2(ic));
  POKE(F64, vp ++, x + y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_f64_sqrt(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  F64 x = PEEK(F64, fp + H1(ic));
  POKE(F64, vp ++, sqrt(x));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i32_add(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U32 x = PEEK(U32, fp + H1(ic));
  U32 y = PEEK(U32, fp + H2(ic));
  POKE(U32, vp ++, x + y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_add(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x + y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_bit_and(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x & y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_bit_not(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, ~ x);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_bit_or(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x | y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_bit_xor(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x ^ y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_clz(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, (U64) clz64(x));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_ctz(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, (U64) ctz64(x));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_is_eq(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x == y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_is_le_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  S64 y = PEEK(S64, fp + H2(ic));
  POKE(Bool, vp ++, x <= y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_is_le_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x <= y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_is_lt_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  S64 y = PEEK(S64, fp + H2(ic));
  POKE(Bool, vp ++, x < y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_is_lt_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(Bool, vp ++, x < y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_mul(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x * y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_mul_full_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S128 x = (S128) PEEK(S64, fp + H1(ic));
  S128 y = (S128) PEEK(S64, fp + H2(ic));
  S128 z = x * y;
  POKE(U64, vp ++, (U64) (U128) z);
  POKE(U64, vp ++, (U64) ((U128) z >> 64));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_mul_full_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U128 x = (U128) PEEK(U64, fp + H1(ic));
  U128 y = (U128) PEEK(U64, fp + H2(ic));
  U128 z = x * y;
  POKE(U64, vp ++, (U64) z);
  POKE(U64, vp ++, (U64) (z >> 64));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_mul_hi_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S128 x = (S128) PEEK(S64, fp + H1(ic));
  S128 y = (S128) PEEK(S64, fp + H2(ic));
  S128 z = x * y;
  POKE(U64, vp ++, (U64) ((U128) z >> 64));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_mul_hi_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U128 x = (U128) PEEK(U64, fp + H1(ic));
  U128 y = (U128) PEEK(U64, fp + H2(ic));
  U128 z = x * y;
  POKE(U64, vp ++, (U64) (z >> 64));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_neg(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, - x);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_rev(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U64, vp ++, rev64(x));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_rol(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(U64, vp ++, rol64(x, y));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_ror(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(U64, vp ++, ror64(x, y));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_shl(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(U64, vp ++, shl64(x, y));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_shr_s(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  S64 x = PEEK(S64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(S64, vp ++, asr64(x, y));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_shr_u(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U8 y = PEEK(U8, fp + H2(ic));
  POKE(U64, vp ++, lsr64(x, y));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_sub(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  U64 y = PEEK(U64, fp + H2(ic));
  POKE(U64, vp ++, x - y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_to_f64_bitcast(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  F64 y = PEEK(F64, &x);
  POKE(F64, vp ++, y);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_to_i32(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U32, vp ++, (U32) x);
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_to_i32_hi(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U32, vp ++, (U32) (x >> 32));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_to_i5(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U8, vp ++, (U8) (x & 0x1f));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode vm_i64_to_i6(Env * ep, L64 * ip, U64 * fp, U64 * vp, U64 * sp, U64 ic) {
  U64 x = PEEK(U64, fp + H1(ic));
  POKE(U8, vp ++, (U8) (x & 0x3f));
  return vm_dispatch(ep, ip, fp, vp, sp);
}

static ExitCode interpret(L64 * ip) {
  Env env = {
    .dispatch = {
      [OP_ABORT] = vm_abort,
      [OP_CALL] = vm_call,
      [OP_CALL_INDIRECT] = vm_call,
      [OP_GOTO] = vm_goto,
      [OP_IF] = vm_if,
      [OP_NOP] = vm_nop,
      [OP_RETURN] = vm_return,
      [OP_SELECT] = vm_select,
      [OP_SHOW_I64] = vm_show_i64,
      [OP_CONST_F32] = vm_const_f32,
      [OP_CONST_F64] = vm_const_f64,
      [OP_CONST_I32] = vm_const_i32,
      [OP_CONST_I64] = vm_const_i64,
      [OP_F32_ADD] = vm_f32_add,
      [OP_F32_SQRT] = vm_f32_sqrt,
      [OP_F64_ADD] = vm_f64_add,
      [OP_F64_SQRT] = vm_f64_sqrt,
      [OP_I32_ADD] = vm_i32_add,
      [OP_I64_ADD] = vm_i64_add,
      [OP_I64_BIT_AND] = vm_i64_bit_and,
      [OP_I64_BIT_NOT] = vm_i64_bit_not,
      [OP_I64_BIT_OR] = vm_i64_bit_or,
      [OP_I64_BIT_XOR] = vm_i64_bit_xor,
      [OP_I64_CLZ] = vm_i64_clz,
      [OP_I64_CTZ] = vm_i64_ctz,
      [OP_I64_IS_EQ] = vm_i64_is_eq,
      [OP_I64_IS_LE_S] = vm_i64_is_le_s,
      [OP_I64_IS_LE_U] = vm_i64_is_le_u,
      [OP_I64_IS_LT_S] = vm_i64_is_lt_s,
      [OP_I64_IS_LT_U] = vm_i64_is_lt_u,
      [OP_I64_MUL] = vm_i64_mul,
      [OP_I64_MUL_FULL_S] = vm_i64_mul_full_s,
      [OP_I64_MUL_FULL_U] = vm_i64_mul_full_u,
      [OP_I64_MUL_HI_S] = vm_i64_mul_hi_s,
      [OP_I64_MUL_HI_U] = vm_i64_mul_hi_u,
      [OP_I64_NEG] = vm_i64_neg,
      [OP_I64_REV] = vm_i64_rev,
      [OP_I64_ROL] = vm_i64_rol,
      [OP_I64_ROR] = vm_i64_ror,
      [OP_I64_SHL] = vm_i64_shl,
      [OP_I64_SHR_S] = vm_i64_shr_s,
      [OP_I64_SHR_U] = vm_i64_shr_u,
      [OP_I64_SUB] = vm_i64_sub,
      [OP_I64_TO_F64_BITCAST] = vm_i64_to_f64_bitcast,
      [OP_I64_TO_I32] = vm_i64_to_i32,
      [OP_I64_TO_I32_HI] = vm_i64_to_i32_hi,
      [OP_I64_TO_I5] = vm_i64_to_i5,
      [OP_I64_TO_I6] = vm_i64_to_i6,
    },
  };

  L64 stub[] = {
    ic_make_hhh_(OP_CONTINUE, 1, 0),
    ic_make_h___(2),
    ic_make_hhh_(OP_LABEL, 0, 0),
    ic_make_h_w_(OP_ABORT, EXIT_CODE_OK),
  };

  U64 ic = PEEK_LE(U64, ip ++);

  ASSERT(H0(ic) == OP_ENTER);
  ASSERT(H1(ic) == 0); // # args
  ASSERT(H3(ic) == 1); // # konts

  const size_t INIT_STACK_SIZE = 256;

  U64 * stack = malloc(sizeof(U64) * INIT_STACK_SIZE);

  if (! stack) {
    return EXIT_CODE_PANIC;
  }

  POKE(L64 *, &stack[0], &stub[0]);

  env.bp = stack;
  env.lp = stack + INIT_STACK_SIZE;

  ExitCode ec =
    vm_dispatch(
        &env,
        ip,
        stack + 1,
        stack + 1,
        stack + 1 + H2(ic) + 1
    );

  free(env.bp);

  return ec;
}
