	.text
	.globl _start

_start:
	cpy i dr0, target_label
	jmp dr0

	cpy tdr0, loc
	add i tdr0, %pcrel(data_word)
	ld q dr2, [tdr0 + 0]

	cpy q3 i dr3, %q3(subroutine)
	cpy q2 i dr3, %q2(subroutine)
	cpy q1 i dr3, %q1(subroutine)
	cpy q0 i dr3, %q0(subroutine)
	call dr3

target_label:
	xor dr0, dr0
	ret

subroutine:
	add dr0, dr1
	ret

	.data
	.p2align 3
data_word:
	.quad 0xDEADBEEFCAFEBABE
	.long 0xDEADBEEF
