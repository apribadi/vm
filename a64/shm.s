.align 2
.global _c_call_0
.global _c_call_1
.global _v_exec_0
.global _v_kont_0

_v_exec_0: // x0 = ep, x1 = lp, x2 = sp, x3 = fn
  mov x10, sp
  str x10, [x0]
  mov sp, x2
  str xzr, [sp, -16]!
  blr x3
  adrp x2, _c_prim_exit@PAGE
  add x2, x2, _c_prim_exit@PAGEOFF
  bl _c_call_0

_v_kont_0: // x0 = ep, x1 = lp, x2 = sp, x3 = rt
  mov sp, x2
  mov lr, x3
  ret

_c_call_0: // x0 = ep, x1 = lp, x2 = fn
  ldr x10, [x0]
  mov x11, x2
  mov x2, sp
  mov x3, lr
  mov sp, x10
  mov lr, xzr
  br x11

_c_call_1: // x0 = ep, x1 = lp, x2 = a0, x3 = fn
  ldr x10, [x0]
  mov x11, x3
  mov x3, sp
  mov x4, lr
  mov sp, x10
  mov lr, xzr
  br x11
