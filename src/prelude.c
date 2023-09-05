#define STATIC_INLINE static inline __attribute__((always_inline))
#define TAIL __attribute__((musttail))

typedef bool Bool;
typedef unsigned char Byte;
typedef float F32;
typedef double F64;
typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

// little-endian

typedef union L16 {
  U16 value;
  Byte b[2];
} L16;

typedef union L32 {
  U32 value;
  Byte b[4];
  struct { L16 h0; L16 h1; };
} L32;

typedef union L64 {
  U64 value;
  Byte b[8];
  struct { L16 h0; L16 h1; L16 h2; L16 h3; };
  struct { L32 w0; L32 w1; };
} L64;

static_assert(sizeof(Bool) == 1);
static_assert(sizeof(Byte) == 1);
static_assert(sizeof(F32) == 4);
static_assert(sizeof(F64) == 8);
static_assert(sizeof(S8) == 1);
static_assert(sizeof(S16) == 2);
static_assert(sizeof(S32) == 4);
static_assert(sizeof(S64) == 8);
static_assert(sizeof(U8) == 1);
static_assert(sizeof(U16) == 2);
static_assert(sizeof(U32) == 4);
static_assert(sizeof(U64) == 8);
static_assert(sizeof(L16) == 2 && alignof(L16) == 2);
static_assert(sizeof(L32) == 4 && alignof(L32) == 4);
static_assert(sizeof(L64) == 8 && alignof(L64) == 8);

STATIC_INLINE Bool get_bool(void * p) {
  Bool x;
  memcpy(&x, p, 1);
  return x;
}

STATIC_INLINE F32 get_f32(void * p) {
  F32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE F64 get_f64(void * p) {
  F64 x;
  memcpy(&x, p, 8);
  return x;
}

STATIC_INLINE U8 get_u8(void * p) {
  U8 x;
  memcpy(&x, p, 1);
  return x;
}

STATIC_INLINE U32 get_u32(void * p) {
  U32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE U64 get_u64(void * p) {
  U64 x;
  memcpy(&x, p, 8);
  return x;
}

STATIC_INLINE void set_bool(void * p, Bool x) {
  memcpy(p, &x, 1);
}

STATIC_INLINE void set_f32(void * p, F32 x) {
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_f64(void * p, F64 x) {
  memcpy(p, &x, 8);
}

STATIC_INLINE void set_u8(void * p, U8 x) {
  memcpy(p, &x, 1);
}

STATIC_INLINE void set_u32(void * p, U32 x) {
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_u64(void * p, U64 x) {
  memcpy(p, &x, 8);
}

STATIC_INLINE F32 get_le_f32(L32 * p) {
  // TODO: byteswap if big endian
  F32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE F64 get_le_f64(L64 * p) {
  // TODO: byteswap if big endian
  F64 x;
  memcpy(&x, p, 8);
  return x;
}

STATIC_INLINE S16 get_le_s16(L16 * p) {
  // TODO: byteswap if big endian
  S16 x;
  memcpy(&x, p, 2);
  return x;
}

STATIC_INLINE U16 get_le_u16(L16 * p) {
  // TODO: byteswap if big endian
  U16 x;
  memcpy(&x, p, 2);
  return x;
}

STATIC_INLINE U32 get_le_u32(L32 * p) {
  // TODO: byteswap if big endian
  U32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE U64 get_le_u64(L64 * p) {
  // TODO: byteswap if big endian
  U64 x;
  memcpy(&x, p, 8);
  return x;
}

STATIC_INLINE void set_le_u16(L16 * p, U16 x) {
  // TODO: byteswap if big endian
  memcpy(p, &x, 2);
}

STATIC_INLINE void set_le_u32(L32 * p, U32 x) {
  // TODO: byteswap if big endian
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_le_u64(L64 * p, U64 x) {
  // TODO: byteswap if big endian
  memcpy(p, &x, 8);
}
