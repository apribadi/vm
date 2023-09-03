enum Op: u16 {
  OP_ABORT,
  OP_BRANCH,
  OP_CALL,
  OP_CALL_INDIRECT,
  OP_CALL_INDIRECT_TAIL,
  OP_CALL_TAIL,
  OP_ENTER,
  OP_EXIT,
  OP_JUMP,
  OP_LABEL,
  OP_NOP,
  OP_RET,
  OP_SHOW_I64,
  OP_CONST_BOOL,
  OP_CONST_F32,
  OP_CONST_F64,
  OP_CONST_I32,
  OP_CONST_I5,
  OP_CONST_I6,
  OP_CONST_I64,
  OP_CONST_V256,
  OP_PRIM_BOOL_AND,
  OP_PRIM_BOOL_IS_EQ,
  OP_PRIM_BOOL_IS_GE,
  OP_PRIM_BOOL_IS_GT,
  OP_PRIM_BOOL_IS_LE,
  OP_PRIM_BOOL_IS_LT,
  OP_PRIM_BOOL_IS_NE,
  OP_PRIM_BOOL_NOT,
  OP_PRIM_BOOL_OR,
  OP_PRIM_F32_ABS,
  OP_PRIM_F32_ADD,
  OP_PRIM_F32_BITS,
  OP_PRIM_F32_DIV,
  OP_PRIM_F32_IS_EQ,
  OP_PRIM_F32_IS_GE,
  OP_PRIM_F32_IS_GT,
  OP_PRIM_F32_IS_LE,
  OP_PRIM_F32_IS_LT,
  OP_PRIM_F32_IS_NE,
  OP_PRIM_F32_MUL,
  OP_PRIM_F32_ROUND,
  OP_PRIM_F32_ROUND_TO_I32,
  OP_PRIM_F32_ROUND_TO_I64,
  OP_PRIM_F32_SQRT,
  OP_PRIM_F32_SUB,
  OP_PRIM_F32_TO_F64,
  OP_PRIM_F64_ABS,
  OP_PRIM_F64_ADD,
  OP_PRIM_F64_BITS,
  OP_PRIM_F64_DIV,
  OP_PRIM_F64_IS_EQ,
  OP_PRIM_F64_IS_GE,
  OP_PRIM_F64_IS_GT,
  OP_PRIM_F64_IS_LE,
  OP_PRIM_F64_IS_LT,
  OP_PRIM_F64_IS_NE,
  OP_PRIM_F64_MUL,
  OP_PRIM_F64_ROUND,
  OP_PRIM_F64_ROUND_TO_I32,
  OP_PRIM_F64_ROUND_TO_I64,
  OP_PRIM_F64_SQRT,
  OP_PRIM_F64_SUB,
  OP_PRIM_F64_TO_F32,
  OP_PRIM_I32_ADD,
  OP_PRIM_I32_BIT_AND,
  OP_PRIM_I32_BIT_NOT,
  OP_PRIM_I32_BIT_OR,
  OP_PRIM_I32_BIT_XOR,
  OP_PRIM_I32_BYTESWAP,
  OP_PRIM_I32_CLZ,
  OP_PRIM_I32_CTZ,
  OP_PRIM_I32_IS_EQ,
  OP_PRIM_I32_IS_GE_S,
  OP_PRIM_I32_IS_GE_U,
  OP_PRIM_I32_IS_GT_S,
  OP_PRIM_I32_IS_GT_U,
  OP_PRIM_I32_IS_LE_S,
  OP_PRIM_I32_IS_LE_U,
  OP_PRIM_I32_IS_LT_S,
  OP_PRIM_I32_IS_LT_U,
  OP_PRIM_I32_IS_NE,
  OP_PRIM_I32_MAX_S,
  OP_PRIM_I32_MAX_U,
  OP_PRIM_I32_MIN_S,
  OP_PRIM_I32_MIN_U,
  OP_PRIM_I32_MUL,
  OP_PRIM_I32_MUL_FULL_S,
  OP_PRIM_I32_MUL_FULL_U,
  OP_PRIM_I32_MUL_HI_S,
  OP_PRIM_I32_MUL_HI_U,
  OP_PRIM_I32_NEG,
  OP_PRIM_I32_POPCOUNT,
  OP_PRIM_I32_ROL,
  OP_PRIM_I32_ROR,
  OP_PRIM_I32_SHL,
  OP_PRIM_I32_SHR_S,
  OP_PRIM_I32_SHR_U,
  OP_PRIM_I32_SUB,
  OP_PRIM_I32_TO_F32_BITS,
  OP_PRIM_I32_TO_I5,
  OP_PRIM_I32_TO_I6,
  OP_PRIM_I32_TO_I64_CONCAT,
  OP_PRIM_I32_TO_I64_S,
  OP_PRIM_I32_TO_I64_U,
  OP_PRIM_I64_ADD,
  OP_PRIM_I64_BIT_AND,
  OP_PRIM_I64_BIT_NOT,
  OP_PRIM_I64_BIT_OR,
  OP_PRIM_I64_BIT_XOR,
  OP_PRIM_I64_BYTESWAP,
  OP_PRIM_I64_CLZ,
  OP_PRIM_I64_CTZ,
  OP_PRIM_I64_IS_EQ,
  OP_PRIM_I64_IS_GE_S,
  OP_PRIM_I64_IS_GE_U,
  OP_PRIM_I64_IS_GT_S,
  OP_PRIM_I64_IS_GT_U,
  OP_PRIM_I64_IS_LE_S,
  OP_PRIM_I64_IS_LE_U,
  OP_PRIM_I64_IS_LT_S,
  OP_PRIM_I64_IS_LT_U,
  OP_PRIM_I64_IS_NE,
  OP_PRIM_I64_MAX_S,
  OP_PRIM_I64_MAX_U,
  OP_PRIM_I64_MIN_S,
  OP_PRIM_I64_MIN_U,
  OP_PRIM_I64_MUL,
  OP_PRIM_I64_MUL_FULL_S,
  OP_PRIM_I64_MUL_FULL_U,
  OP_PRIM_I64_MUL_HI_S,
  OP_PRIM_I64_MUL_HI_U,
  OP_PRIM_I64_NEG,
  OP_PRIM_I64_POPCOUNT,
  OP_PRIM_I64_ROL,
  OP_PRIM_I64_ROR,
  OP_PRIM_I64_SHL,
  OP_PRIM_I64_SHR_S,
  OP_PRIM_I64_SHR_U,
  OP_PRIM_I64_SUB,
  OP_PRIM_I64_TO_F64_BITS,
  OP_PRIM_I64_TO_I32,
  OP_PRIM_I64_TO_I32_HI,
  OP_PRIM_I64_TO_I5,
  OP_PRIM_I64_TO_I6,
  OP_COUNT,
};

enum Ty: u8 {
  TY_BOOL,
  TY_F32,
  TY_F64,
  TY_I32,
  TY_I5,
  TY_I6,
  TY_I64,
  TY_V256,
};

STATIC_INLINE u64 iw_make_obbb(enum Op op, u16 b1, u16 b2, u16 b3) {
  return
      (u64) ((u64) op      )
    | (u64) ((u64) b1 << 16)
    | (u64) ((u64) b2 << 32)
    | (u64) ((u64) b3 << 48);
}

STATIC_INLINE u64 iw_make_o___(enum Op op) {
  return iw_make_obbb(op, 0, 0, 0);
}

STATIC_INLINE u64 iw_make_ob__(enum Op op, u16 b1) {
  return iw_make_obbb(op, b1, 0, 0);
}

STATIC_INLINE u64 iw_make_obb_(enum Op op, u16 b1, u16 b2) {
  return iw_make_obbb(op, b1, b2, 0);
}

STATIC_INLINE u16 iw_b0(u64 iw) {
  return (u16) iw;
}

STATIC_INLINE u16 iw_b1(u64 iw) {
  return (u16) (iw >> 16);
}

STATIC_INLINE u16 iw_b2(u64 iw) {
  return (u16) (iw >> 32);
}

STATIC_INLINE u16 iw_b3(u64 iw) {
  return (u16) (iw >> 48);
}

STATIC_INLINE u32 iw_c1(u64 iw) {
  return (u16) (iw >> 32);
}
