.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	movw r0, #20
	bl malloc
	mov r9, r0
	str r9, [fp, #-40]
	movw r0, #4
	ldr r9, [fp, #-40]
	str r0, [r9]
	movw r0, #1
	ldr r9, [fp, #-40]
	str r0, [r9, #4]
	movw r0, #2
	ldr r9, [fp, #-40]
	str r0, [r9, #8]
	movw r0, #3
	ldr r9, [fp, #-40]
	str r0, [r9, #12]
	movw r0, #4
	ldr r9, [fp, #-40]
	str r0, [r9, #16]
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	mov r9, r0
	str r9, [fp, #-44]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r10, [fp, #-44]
	sub r9, r10, #1
	str r9, [fp, #-48]
	ldr r9, [fp, #-40]
	mov r4, r9
	ldr r10, [fp, #-40]
	ldr r9, [r10]
	str r9, [fp, #-52]
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-52]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-48]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
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
