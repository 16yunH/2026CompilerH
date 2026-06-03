.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	bl getint
	mov r9, r0
	str r9, [fp, #-40]
	mov r0, #10
	mov r1, #0
	mov r9, r0
	str r9, [fp, #-52]
	mov r9, r1
	str r9, [fp, #-44]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-52]
	cmp r9, r0
	bgt main$L103
main$L104:
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L107
main$L108:
	mov r0, #4
	mov r1, r0
main$L109:
	ldr r9, [fp, #-52]
	sub r0, r9, #1
	mov r9, r0
	str r9, [fp, #-52]
	mov r9, r1
	str r9, [fp, #-44]
	b main$L102
main$L107:
	movw r0, #3
	ldr r9, [fp, #-52]
	mul r0, r0, r9
	add r9, r0, #2
	str r9, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r0, r9
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-48]
	mov r1, r9
	b main$L109

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
