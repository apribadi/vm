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

typedef struct { uint16_t value; } b16;
typedef struct { uint32_t value; } b32;

typedef struct {
  union {
    uint64_t value;
    struct { b16 h0; b16 h1; b16 h2; b16 h3; };
    struct { b32 w0; b32 w1; };
  };
} b64;

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



////////

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

STATIC_INLINE void set_u32(void * p, u32 x) {
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_u64(void * p, u64 x) {
  memcpy(p, &x, 8);
}

STATIC_INLINE void set_f32(void * p, f32 x) {
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_f64(void * p, f64 x) {
  memcpy(p, &x, 8);
}

STATIC_INLINE s16 get_le_s16(void * p) {
  // TODO: maybe byteswap
  s16 x;
  memcpy(&x, p, 2);
  return x;
}

STATIC_INLINE u16 get_le_u16(void * p) {
  // TODO: maybe byteswap
  u16 x;
  memcpy(&x, p, 2);
  return x;
}

STATIC_INLINE u32 get_le_u32(void * p) {
  // TODO: maybe byteswap
  u32 x;
  memcpy(&x, p, 4);
  return x;
}

STATIC_INLINE u64 get_le_u64(void * p) {
  // TODO: maybe byteswap
  u64 x;
  memcpy(&x, p, 8);
  return x;
}

////////


STATIC_INLINE b16 le_u16_to_b16(u16 x) {
  // TODO: maybe byteswap
  b16 y;
  memcpy(&y, &x, 2);
  return y;
}

STATIC_INLINE b32 le_u32_to_b32(u32 x) {
  // TODO: maybe byteswap
  b32 y;
  memcpy(&y, &x, 4);
  return y;
}

STATIC_INLINE b64 le_u64_to_b64(u64 x) {
  // TODO: maybe byteswap
  b64 y;
  memcpy(&y, &x, 8);
  return y;
}
