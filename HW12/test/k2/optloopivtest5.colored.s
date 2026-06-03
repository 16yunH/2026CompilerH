.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #28
	add fp, sp, #60
	bl getint
	mov r10, #10
	str r10, [fp, #-52]
	mov r10, #0
	str r10, [fp, #-40]
	movw r1, #0
	cmp r0, r1
	bgt main$L107
main$L108:
	mov r10, #4
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
main$L109:
	ldr r9, [fp, #-48]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-48]
	mov r0, r9
	sub sp, fp, #60
	add sp, sp, #28
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-40]
	mov r0, r9
main$L104:
	movw r1, #0
	ldr r9, [fp, #-56]
	cmp r9, r1
	bgt main$L105
main$L106:
	mov r10, r0
	str r10, [fp, #-48]
	b main$L109
main$L105:
	movw r0, #3
	ldr r10, [fp, #-56]
	mul r0, r0, r10
	ldr r9, [fp, #-56]
	sub r10, r9, #1
	str r10, [fp, #-60]
	add r10, r0, #2
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-44]
	mov r0, r9
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
