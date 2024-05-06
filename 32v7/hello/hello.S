	.text
	.global _start
_start:
	mov r0, #1		// stdout
	ldr r1, =message
	mov r2, #messagelen
	mov r7, #4
	svc 0

	mov r0, #42
	mov r7, #1
	svc 0

	.data
message:
	.ascii "Hello, World!\n"
	messagelen == .-message
