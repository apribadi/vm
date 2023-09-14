typedef enum OpCode : U16 {
  OP_ABORT,
  OP_CALL,
  OP_CALL_INDIRECT,
  OP_CALL_TAIL,
  OP_CALL_TAIL_INDIRECT,
  OP_CONTINUE,
  OP_ENTER,
  OP_GOTO,
  OP_IF,
  OP_LABEL,
  OP_NOP,
  OP_RETURN,
  OP_SWITCH,
  OP_SHOW_I64,
  OP_CONST_BOOL,
  OP_CONST_F32,
  OP_CONST_F64,
  OP_CONST_I32,
  OP_CONST_I5,
  OP_CONST_I6,
  OP_CONST_I64,
  OP_CONST_V256,
  OP_BOOL_AND,
  OP_BOOL_IS_EQ,
  OP_BOOL_IS_LE,
  OP_BOOL_IS_LT,
  OP_BOOL_NOT,
  OP_BOOL_OR,
  OP_F32_ABS,
  OP_F32_ADD,
  OP_F32_DIV,
  OP_F32_IS_EQ,
  OP_F32_IS_LE,
  OP_F32_IS_LT,
  OP_F32_MUL,
  OP_F32_ROUND,
  OP_F32_SQRT,
  OP_F32_SUB,
  OP_F32_TO_F64,
  OP_F32_TO_I32_BITCAST,
  OP_F32_TO_I32_ROUND,
  OP_F32_TO_I64_ROUND,
  OP_F64_ABS,
  OP_F64_ADD,
  OP_F64_DIV,
  OP_F64_IS_EQ,
  OP_F64_IS_LE,
  OP_F64_IS_LT,
  OP_F64_MUL,
  OP_F64_ROUND,
  OP_F64_SQRT,
  OP_F64_SUB,
  OP_F64_TO_F32,
  OP_F64_TO_I32_ROUND,
  OP_F64_TO_I64_BITCAST,
  OP_F64_TO_I64_ROUND,
  OP_I32_ADD,
  OP_I32_BIT_AND,
  OP_I32_BIT_NOT,
  OP_I32_BIT_OR,
  OP_I32_BIT_XOR,
  OP_I32_CLZ,
  OP_I32_CTZ,
  OP_I32_IS_EQ,
  OP_I32_IS_LE_S,
  OP_I32_IS_LE_U,
  OP_I32_IS_LT_S,
  OP_I32_IS_LT_U,
  OP_I32_MUL,
  OP_I32_MUL_FULL_S,
  OP_I32_MUL_FULL_U,
  OP_I32_NEG,
  OP_I32_REV,
  OP_I32_ROL,
  OP_I32_ROR,
  OP_I32_SHL,
  OP_I32_SHR_S,
  OP_I32_SHR_U,
  OP_I32_SUB,
  OP_I32_TO_F32_BITCAST,
  OP_I32_TO_I5,
  OP_I32_TO_I6,
  OP_I32_TO_I64_CONCAT,
  OP_I32_TO_I64_S,
  OP_I32_TO_I64_U,
  OP_I64_ADD,
  OP_I64_BIT_AND,
  OP_I64_BIT_NOT,
  OP_I64_BIT_OR,
  OP_I64_BIT_XOR,
  OP_I64_CLZ,
  OP_I64_CTZ,
  OP_I64_IS_EQ,
  OP_I64_IS_LE_S,
  OP_I64_IS_LE_U,
  OP_I64_IS_LT_S,
  OP_I64_IS_LT_U,
  OP_I64_MUL,
  OP_I64_MUL_FULL_S,
  OP_I64_MUL_FULL_U,
  OP_I64_MUL_HI_S,
  OP_I64_MUL_HI_U,
  OP_I64_NEG,
  OP_I64_REV,
  OP_I64_ROL,
  OP_I64_ROR,
  OP_I64_SELECT,
  OP_I64_SHL,
  OP_I64_SHR_S,
  OP_I64_SHR_U,
  OP_I64_SUB,
  OP_I64_TO_F64_BITCAST,
  OP_I64_TO_I32,
  OP_I64_TO_I32_HI,
  OP_I64_TO_I5,
  OP_I64_TO_I6,
  OP_COUNT,
} OpCode;

typedef enum TyCode : U16 {
  TY_BOOL,
  TY_F32,
  TY_F64,
  TY_FUNREF,
  TY_I32,
  TY_I5,
  TY_I6,
  TY_I64,
  TY_I8,
  TY_V256,
} TyCode;

static inline L64 ic_make_hhhh(U16 h0, U16 h1, U16 h2, U16 h3) {
  U64 x =
    (U64) h0
    | (U64) ((U64) h1 << 16)
    | (U64) ((U64) h2 << 32)
    | (U64) ((U64) h3 << 48);
  L64 r;
  POKE_LE(U64, &r, x);
  return r;
}

static inline L64 ic_make_hhw_(U16 h0, U16 h1, U32 w1) {
  U64 x =
    (U64) h0
    | (U64) ((U64) h1 << 16)
    | (U64) ((U64) w1 << 32);
  L64 r;
  POKE_LE(U64, &r, x);
  return r;
}

static inline L64 ic_make_d___(U64 x) {
  L64 r;
  POKE_LE(U64, &r, x);
  return r;
}

static inline L64 ic_make_h___(U16 h0) {
  return ic_make_hhhh(h0, 0, 0, 0);
}

static inline L64 ic_make_hh__(U16 h0, U16 h1) {
  return ic_make_hhhh(h0, h1, 0, 0);
}

static inline L64 ic_make_hhh_(U16 h0, U16 h1, U16 h2) {
  return ic_make_hhhh(h0, h1, h2, 0);
}

static inline L64 ic_make_h_w_(U16 h0, U32 w1) {
  return ic_make_hhw_(h0, 0, w1);
}
