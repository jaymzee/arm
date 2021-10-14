	.text
	.global _start
_start:
	mov r0, #1		// stdout
	ldr r1, =message
	ldr r2, =messagelen
	ldr r3, =foo
	str r2, [r3]
	str r2, [r3, #4]
	mov r7, #4
	svc 0

	mov r0, r2
	add r0, r2, r1
	add r0, r2, r1, lsl #1
	add r0, r2, r1, lsl #31
	add r0, r2, r1, lsr #1
	add r0, r2, r1, lsr #32
	add r0, r2, r1, asr #32
	add r0, r2, r1, ror #31
	mov r0, #42
	mov r7, #1
	svc 0

	.data
message:
	.ascii "Hello, World!\n"
	messagelen == .-message
foo:
	.space 32, 0
