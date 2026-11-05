	.text
	.globl _start

_start:
	jmp rel eq dr0, dr1, forward
	jmp rel ne dr0, dr1, forward
	jmp rel lt dr0, dr1, forward
	jmp rel ge dr0, dr1, forward
	jmp rel ltu dr0, dr1, forward
	jmp rel geu dr0, dr1, forward
	jmp rel al forward

forward:
	setcc eq dr2, dr3
	setcc ne dr2, dr3
	setcc lt dr2, dr3
	setcc ge dr2, dr3
	setcc ltu dr2, dr3
	setcc geu dr2, dr3

	jmp rel lt dr4, dr5, _start
	ret
