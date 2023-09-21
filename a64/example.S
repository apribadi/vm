.align 2
.global _c_call_0
.global _c_call_1
.global _v_exec_0
.global _v_kont_0
.global _v_main

_v_exec_0: // x0 = ep, x1 = lp, x2 = sp, x3 = fn
  mov x10, sp
  str x10, [x0]
  sub sp, x2, 16
  mov fp, 0
  blr x3
  adrp x2, _c_prim_exit@PAGE
  add x2, x2, _c_prim_exit@PAGEOFF
  bl _c_call_0

_v_kont_0: // x0 = ep, x1 = lp, x2 = sp
  mov sp, x2
  ldp fp, lr, [sp]
  ret

_c_call_0: // x0 = ep, x1 = lp, x2 = fn
  stp fp, lr, [sp]
  ldr x10, [x0]
  mov x11, x2
  mov x2, sp
  mov sp, x10
  mov fp, 0
  mov lr, 0
  br x11

_c_call_1: // x0 = ep, x1 = lp, x2 = fn, x3 = a0
  stp fp, lr, [sp]
  ldr x10, [x0]
  mov x11, x2
  mov x2, sp
  mov sp, x10
  mov fp, 0
  mov lr, 0
  br x11

_v_main:
  stp fp, lr, [sp], -16
  add fp, sp, 16
  adrp x2, _c_prim_show@PAGE
  add x2, x2, _c_prim_show@PAGEOFF
  mov x3, 101
  bl _c_call_1
  adrp x2, _c_prim_hello@PAGE
  add x2, x2, _c_prim_hello@PAGEOFF
  bl _c_call_0
  ldp fp, lr, [sp, 16]!
  ret
