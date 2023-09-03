typedef unsigned char byte;

// TODO: big endian support

STATIC_INLINE u8 get_i8(byte * p) {
  return p[0];
}

STATIC_INLINE u16 get_i16(byte * p) {
  u16 r;
  memcpy(&r, p, 2);
  return r;
}

STATIC_INLINE u32 get_i32(byte * p) {
  u32 r;
  memcpy(&r, p, 4);
  return r;
}

STATIC_INLINE u64 get_i64(byte * p) {
  u64 r;
  memcpy(&r, p, 8);
  return r;
}

STATIC_INLINE void set_i8(byte * p, u8 x) {
  memcpy(p, &x, 1);
}

STATIC_INLINE void set_i16(byte * p, u16 x) {
  memcpy(p, &x, 2);
}

STATIC_INLINE void set_i32(byte * p, u32 x) {
  memcpy(p, &x, 4);
}

STATIC_INLINE void set_i64(byte * p, u64 x) {
  memcpy(p, &x, 8);
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
