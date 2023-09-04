#define STATIC_INLINE static inline __attribute__((always_inline))
#define TAIL __attribute__((musttail))

typedef unsigned char byte;
typedef float f32;
typedef double f64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef union b16 {
  u16 value;
} b16;

typedef union b32 {
  u32 value;
  b16 h[2];
  struct { b16 h0; b16 h1; };
} b32;

typedef union b64 {
  u64 value;
  byte b[8];
  b16 h[4];
  b32 w[2];
  struct { b16 h0; b16 h1; b16 h2; b16 h3; };
  struct { b32 w0; b32 w1; };
} b64;

static_assert(sizeof(bool) == 1);
static_assert(sizeof(byte) == 1);
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);
static_assert(sizeof(s8) == 1);
static_assert(sizeof(s16) == 2);
static_assert(sizeof(s32) == 4);
static_assert(sizeof(s64) == 8);
static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);
static_assert(sizeof(b16) == 2 && alignof(b16) == 2);
static_assert(sizeof(b32) == 4 && alignof(b32) == 4);
static_assert(sizeof(b64) == 8 && alignof(b64) == 8);

STATIC_INLINE bool get_bool(void * p) {
  bool x;
  memcpy(&x, p, 1);
  return x;
}

STATIC_INLINE f32 get_f32(void * p) {
  f32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE f64 get_f64(void * p) {
  f64 x;
  memcpy(&x, p, 8);
  return x;
}

STATIC_INLINE u8 get_u8(void * p) {
  u8 x;
  memcpy(&x, p, 1);
  return x;
}

STATIC_INLINE u32 get_u32(void * p) {
  u32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE u64 get_u64(void * p) {
  u64 x;
  memcpy(&x, p, 8);
  return x;
}

STATIC_INLINE void set_bool(void * p, bool x) {
  memcpy(p, &x, 1);
}

STATIC_INLINE void set_f32(void * p, f32 x) {
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_f64(void * p, f64 x) {
  memcpy(p, &x, 8);
}

STATIC_INLINE void set_u8(void * p, u8 x) {
  memcpy(p, &x, 1);
}

STATIC_INLINE void set_u32(void * p, u32 x) {
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_u64(void * p, u64 x) {
  memcpy(p, &x, 8);
}

STATIC_INLINE f32 get_le_f32(b32 * p) {
  // TODO: byteswap if big endian
  f32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE f64 get_le_f64(b64 * p) {
  // TODO: byteswap if big endian
  f64 x;
  memcpy(&x, p, 8);
  return x;
}

STATIC_INLINE s16 get_le_s16(b16 * p) {
  // TODO: byteswap if big endian
  s16 x;
  memcpy(&x, p, 2);
  return x;
}

STATIC_INLINE u16 get_le_u16(b16 * p) {
  // TODO: byteswap if big endian
  u16 x;
  memcpy(&x, p, 2);
  return x;
}

STATIC_INLINE u32 get_le_u32(b32 * p) {
  // TODO: byteswap if big endian
  u32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE u64 get_le_u64(b64 * p) {
  // TODO: byteswap if big endian
  u64 x;
  memcpy(&x, p, 8);
  return x;
}

STATIC_INLINE void set_le_u16(b16 * p, u16 x) {
  // TODO: byteswap if big endian
  memcpy(p, &x, 2);
}

STATIC_INLINE void set_le_u32(b32 * p, u32 x) {
  // TODO: byteswap if big endian
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_le_u64(b64 * p, u64 x) {
  // TODO: byteswap if big endian
  memcpy(p, &x, 8);
}
