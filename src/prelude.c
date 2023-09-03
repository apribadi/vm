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

typedef struct { uint16_t value; } w16;
typedef struct { uint32_t value; } w32;

typedef struct {
  union {
    uint64_t value;
    struct { w16 h0; w16 h1; w16 h2; w16 h3; };
    struct { w32 w0; w32 w1; };
  };
} w64;

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
static_assert(sizeof(w16) == 2);
static_assert(sizeof(w32) == 4);
static_assert(sizeof(w64) == 8 && alignof(w64) == 8);



////////

STATIC_INLINE s16 le_s16(void * p) {
  // TODO: maybe byteswap
  s16 x;
  memcpy(&x, p, 2);
  return x;
}

STATIC_INLINE u16 le_u16(void * p) {
  // TODO: maybe byteswap
  u16 x;
  memcpy(&x, p, 2);
  return x;
}

STATIC_INLINE u32 le_u32(void * p) {
  // TODO: maybe byteswap
  u32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE u64 le_u64(void * p) {
  // TODO: maybe byteswap
  u64 x;
  memcpy(&x, p, 8);
  return x;
}

////////


STATIC_INLINE u16 le_w16_to_u16(w16 x) {
  // TODO: maybe byteswap
  u16 y;
  memcpy(&y, &x, 2);
  return y;
}

STATIC_INLINE u32 le_w32_to_u32(w32 x) {
  // TODO: maybe byteswap
  u32 y;
  memcpy(&y, &x, 4);
  return y;
}

STATIC_INLINE u64 le_w64_to_u64(w64 x) {
  // TODO: maybe byteswap
  u64 y;
  memcpy(&y, &x, 8);
  return y;
}

STATIC_INLINE f32 le_w32_to_f32(w32 x) {
  // TODO: maybe byteswap
  f32 y;
  memcpy(&y, &x, 4);
  return y;
}

STATIC_INLINE f64 le_w64_to_f64(w64 x) {
  // TODO: maybe byteswap
  f64 y;
  memcpy(&y, &x, 8);
  return y;
}

STATIC_INLINE w16 le_u16_to_w16(u16 x) {
  // TODO: maybe byteswap
  w16 y;
  memcpy(&y, &x, 2);
  return y;
}

STATIC_INLINE w32 le_u32_to_w32(u32 x) {
  // TODO: maybe byteswap
  w32 y;
  memcpy(&y, &x, 4);
  return y;
}

STATIC_INLINE w64 le_u64_to_w64(u64 x) {
  // TODO: maybe byteswap
  w64 y;
  memcpy(&y, &x, 8);
  return y;
}

STATIC_INLINE f64 bitcast_u64_to_f64(u64 x) {
  f64 y;
  memcpy(&y, &x, 8);
  return y;
}

STATIC_INLINE f32 bitcast_u32_to_f32(u32 x) {
  f32 y;
  memcpy(&y, &x, 4);
  return y;
}

STATIC_INLINE u64 bitcast_f64_to_u64(f64 x) {
  u64 y;
  memcpy(&y, &x, 8);
  return y;
}

STATIC_INLINE u32 bitcast_f32_to_u32(f32 x) {
  u32 y;
  memcpy(&y, &x, 4);
  return y;
}
