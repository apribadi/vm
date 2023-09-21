.align 2
.global _c_call_0
.global _c_call_1
.global _v_exec_0
.global _v_kont_0
.global _v_main

_v_exec_0: // x0 = ep, x1 = sp, x2 = fn
  mov x10, sp
  str x10, [x0, 8]
  sub sp, x1, 16
  mov fp, xzr
  blr x2
  adrp x1, _c_prim_exit@PAGE
  add x1, x1, _c_prim_exit@PAGEOFF
  bl _c_call_0

_v_kont_0: // x0 = ep, x1 = sp
  mov sp, x1
  ldp fp, lr, [sp]
  ret

_c_call_0: // x0 = ep, x1 = fn
  stp fp, lr, [sp]
  ldr x10, [x0, 8]
  mov x11, x1
  mov x1, sp
  mov sp, x10
  mov fp, xzr
  mov lr, xzr
  br x11

_c_call_1: // x0 = ep, x1 = fn, x2 = a0
  stp fp, lr, [sp]
  ldr x10, [x0, 8]
  mov x11, x1
  mov x1, sp
  mov sp, x10
  mov fp, xzr
  mov lr, xzr
  br x11

_v_main:
  stp fp, lr, [sp], -16
  add fp, sp, 16
  adrp x1, _c_prim_show@PAGE
  add x1, x1, _c_prim_show@PAGEOFF
  mov x2, 101
  bl _c_call_1
  adrp x1, _c_prim_hello@PAGE
  add x1, x1, _c_prim_hello@PAGEOFF
  bl _c_call_0
  ldp fp, lr, [sp, 16]!
  ret
