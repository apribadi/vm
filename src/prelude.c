#define STATIC_INLINE static inline __attribute__((always_inline))
#define TAIL __attribute__((musttail))

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
