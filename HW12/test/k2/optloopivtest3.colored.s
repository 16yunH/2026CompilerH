.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #36
	add fp, sp, #68
	mov r9, #0
	str r9, [fp, #-40]
	bl getint
	mov r9, r0
	str r9, [fp, #-52]
	bl getint
	mov r9, r0
	str r9, [fp, #-64]
	ldr r10, [fp, #-52]
	mov r9, r10
	str r9, [fp, #-56]
	ldr r10, [fp, #-40]
	mov r9, r10
	str r9, [fp, #-44]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-56]
	cmp r9, r0
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #68
	add sp, sp, #36
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r10, [fp, #-56]
	ldr r12, [fp, #-64]
	sub r9, r10, r12
	str r9, [fp, #-60]
	movw r0, #8
	ldr r9, [fp, #-60]
	mul r0, r0, r9
	add r9, r0, #7
	str r9, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r0, r9
	bl putint
	movw r0, #32
	bl putch
	ldr r10, [fp, #-60]
	mov r9, r10
	str r9, [fp, #-56]
	ldr r10, [fp, #-48]
	mov r9, r10
	str r9, [fp, #-44]
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
