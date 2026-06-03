.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	bl getint
	mov r10, #10
	str r10, [fp, #-44]
	mov r10, #0
	str r10, [fp, #-40]
	movw r1, #0
	cmp r0, r1
	bgt main$L107
main$L108:
	mov r5, #4
main$L109:
	mov r0, r5
	bl putint
	movw r0, #10
	bl putch
	mov r0, r5
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	ldr r9, [fp, #-44]
	mov r4, r9
	ldr r9, [fp, #-40]
	mov r5, r9
main$L104:
	movw r0, #0
	cmp r4, r0
	bgt main$L105
main$L106:
	b main$L109
main$L105:
	movw r0, #3
	mul r0, r0, r4
	sub r4, r4, #1
	add r5, r0, #2
	mov r0, r5
	bl putint
	movw r0, #32
	bl putch
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
