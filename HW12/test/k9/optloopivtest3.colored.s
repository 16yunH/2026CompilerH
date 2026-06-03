.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r4, #0
	bl getint
	mov r5, r0
	bl getint
	mov r9, r0
	str r9, [fp, #-44]
	mov r9, r5
	str r9, [fp, #-40]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r4
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-40]
	ldr r10, [fp, #-44]
	sub r5, r9, r10
	movw r0, #8
	mul r0, r0, r5
	add r4, r0, #7
	mov r0, r4
	bl putint
	movw r0, #32
	bl putch
	mov r9, r5
	str r9, [fp, #-40]
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
