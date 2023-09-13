typedef bool Bool;
typedef float F32;
typedef double F64;
typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;
typedef __int128_t S128;
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef __uint128_t U128;

typedef union L16 {
  U16 value;
} L16;

typedef union L32 {
  U32 value;
  struct { L16 h0; L16 h1; };
} L32;

typedef union L64 {
  U64 value;
  struct { L16 h0; L16 h1; L16 h2; L16 h3; };
  struct { L32 w0; L32 w1; };
} L64;

static_assert(sizeof(Bool) == 1);
static_assert(sizeof(F32) == 4);
static_assert(sizeof(F64) == 8);
static_assert(sizeof(S8) == 1);
static_assert(sizeof(S16) == 2);
static_assert(sizeof(S32) == 4);
static_assert(sizeof(S64) == 8);
static_assert(sizeof(S128) == 16);
static_assert(sizeof(U8) == 1);
static_assert(sizeof(U16) == 2);
static_assert(sizeof(U32) == 4);
static_assert(sizeof(U64) == 8);
static_assert(sizeof(U128) == 16);
static_assert(sizeof(L16) == 2 && alignof(L16) == 2);
static_assert(sizeof(L32) == 4 && alignof(L32) == 4);
static_assert(sizeof(L64) == 8 && alignof(L64) == 8);

#define PEEK(T, P) \
  ({ \
    T _x; \
    memcpy(&_x, (P), sizeof(T)); \
    _x; \
  })

#define POKE(T, P, X) \
  do { \
    T _x = (X); \
    memcpy((P), &_x, sizeof(T)); \
  } while (0)

// TODO: byteswap on big-endian systems

#define PEEK_LE(T, P) \
  ({ \
    T _x; \
    memcpy(&_x, (P), sizeof(T)); \
    _x; \
  })

// TODO: byteswap on big-endian systems

#define POKE_LE(T, P, X) \
  do { \
    T _x = (X); \
    memcpy((P), &_x, sizeof(T)); \
  } while (0)

static inline U32 W0(U64 x) {
  return (U32) x;
}

static inline U32 W1(U64 x) {
  return (U32) (x >> 32);
}

static inline U16 H0(U64 x) {
  return (U16) x;
}

static inline U16 H1(U64 x) {
  return (U16) (x >> 16);
}

static inline U16 H2(U64 x) {
  return (U16) (x >> 32);
}

static inline U16 H3(U64 x) {
  return (U16) (x >> 48);
}

static inline U16 HI(U64 x, int i) {
  return (U16) (x >> 16 * i);
}

static inline U8 B2(U64 x) {
  return (U8) (x >> 16);
}

static inline U8 B3(U64 x) {
  return (U8) (x >> 24);
}

static inline int clz64(U64 x) {
  return x ? __builtin_clzll(x) : 64;
}

static inline int ctz64(U64 x) {
  return x ? __builtin_ctzll(x) : 64;
}

static inline U64 rev64(U64 x) {
  return __builtin_bswap64(x);
}

static inline U64 rol64(U64 x, int k) {
  return x << (k & 0x3f) | x >> (- (unsigned int) k & 0x3f);
}

static inline U64 ror64(U64 x, int k) {
  return x >> (k & 0x3f) | x << (- (unsigned int) k & 0x3f);
}

static inline U64 shl64(U64 x, int k) {
  return x << (k & 0x3f);
}

static inline S64 asr64(S64 x, int k) {
  return x >> (k & 0x3f);
}

static inline U64 lsr64(U64 x, int k) {
  return x >> (k & 0x3f);
}
