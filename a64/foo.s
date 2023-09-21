.align 2
.global _foo

_foo:
  mov x2, 101
  adrp x3, _c_prim_show@PAGE
  add x3, x3, _c_prim_show@PAGEOFF
  bl _c_call_1
