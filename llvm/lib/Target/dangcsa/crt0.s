    .section .text._reset,"ax",@progbits
    .globl _rst
    .type _rst,@function
    .globl _reset
    .type _reset,@function
_rst:
_reset:
    cpy i sp, __stack_top

    cpy i dr0, __bss_start
    cpy i dr1, __bss_end
    cpy i dr2, 0

.Lbss_loop:
    jmp rel geu dr0, dr1, .Lbss_done
    st q [dr0 + 0], dr2
    add i dr0, 8
    jmp rel al .Lbss_loop

.Lbss_done:
    cpy i dr10, __init_array_start
    cpy i dr11, __init_array_end

.Linit_loop:
    jmp rel geu dr10, dr11, .Linit_done
    ld q dr4, [dr10 + 0]
    call dr4
    add i dr10, 8
    jmp rel al .Linit_loop

.Linit_done:
    cpy i dr0, __argc
    cpy i dr1, __argv

    cpy i dr4, main
    call dr4

_halt:
    jmp rel al _halt

.Lreset_end:
    .size _reset, .Lreset_end-_reset

    .section .rodata._argv,"a",@progbits
    .p2align 3
.Largv0:
    .asciz "dangcsa"
    .p2align 3
.Largv1:
    .asciz "test"
    .p2align 3
    .globl __argv
__argv:
    .quad .Largv0
    .quad .Largv1
    .quad 0
