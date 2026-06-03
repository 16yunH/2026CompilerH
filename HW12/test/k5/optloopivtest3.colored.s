.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	mov r9, #0
	str r9, [fp, #-40]
	bl getint
	mov r4, r0
	bl getint
	mov r9, r0
	str r9, [fp, #-52]
	mov r9, r4
	str r9, [fp, #-48]
	ldr r9, [fp, #-40]
	mov r4, r9
main$L102:
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r4
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-52]
	sub r4, r9, r10
	movw r0, #8
	mul r0, r0, r4
	add r9, r0, #7
	str r9, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	movw r0, #32
	bl putch
	mov r9, r4
	str r9, [fp, #-48]
	ldr r9, [fp, #-44]
	mov r4, r9
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
