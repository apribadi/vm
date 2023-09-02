typedef unsigned char byte;

STATIC_INLINE u8 get_i8(byte * p) {
  return (u8) p[0];
}

STATIC_INLINE u16 get_i16(byte * p) {
  return
      (u16) p[0]
    | (u16) p[1] << 8;
}

STATIC_INLINE u32 get_i32(byte * p) {
  return
      (u32) p[0]
    | (u32) p[1] << 8
    | (u32) p[2] << 16
    | (u32) p[3] << 24;
}

STATIC_INLINE u64 get_i64(byte * p) {
  return
      (u64) p[0]
    | (u64) p[1] << 8 * 1
    | (u64) p[2] << 8 * 2
    | (u64) p[3] << 8 * 3
    | (u64) p[4] << 8 * 4
    | (u64) p[5] << 8 * 5
    | (u64) p[6] << 8 * 6
    | (u64) p[7] << 8 * 7;
}

STATIC_INLINE void set_i8(byte * p, u8 x) {
  p[0] = x;
}

STATIC_INLINE void set_i16(byte * p, u16 x) {
  p[0] = x;
  p[1] = x >> 8 * 1;
}

STATIC_INLINE void set_i32(byte * p, u32 x) {
  p[0] = x;
  p[1] = x >> 8 * 1;
  p[2] = x >> 8 * 2;
  p[3] = x >> 8 * 3;
}

STATIC_INLINE void set_i64(byte * p, u64 x) {
  p[0] = x;
  p[1] = x >> 8 * 1;
  p[2] = x >> 8 * 2;
  p[3] = x >> 8 * 3;
  p[4] = x >> 8 * 4;
  p[5] = x >> 8 * 5;
  p[6] = x >> 8 * 6;
  p[7] = x >> 8 * 7;
}

STATIC_INLINE u8 pop_i8(byte ** p) {
  u8 r = get_i8(*p);
  *p += 1;
  return r;
}

STATIC_INLINE u16 pop_i16(byte ** p) {
  u16 r = get_i16(*p);
  *p += 2;
  return r;
}

STATIC_INLINE u32 pop_i32(byte ** p) {
  u32 r = get_i32(*p);
  *p += 4;
  return r;
}

STATIC_INLINE u64 pop_i64(byte ** p) {
  u64 r = get_i64(*p);
  *p += 8;
  return r;
}

STATIC_INLINE void put_i8(byte ** p, u8 x) {
  set_i8(*p, x);
  *p += 1;
}

STATIC_INLINE void put_i16(byte ** p, u16 x) {
  set_i16(*p, x);
  *p += 2;
}

STATIC_INLINE void put_i32(byte ** p, u32 x) {
  set_i32(*p, x);
  *p += 4;
}

STATIC_INLINE void put_i64(byte ** p, u64 x) {
  set_i64(*p, x);
  *p += 8;
}
