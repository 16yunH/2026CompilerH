.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r0, [r0]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #28
	add fp, sp, #60
	movw r0, #8
	bl malloc
	mov r9, r0
	str r9, [fp, #-56]
	mov r9, #3
	str r9, [fp, #-44]
	movw r0, #20
	bl malloc
	mov r4, r0
	movw r0, #4
	str r0, [r4]
	ldr r9, [fp, #-56]
	mov r2, r9
	ldr r9, [fp, #-56]
	add r1, r9, #4
	ldr r9, [fp, #-56]
	mov r0, r9
	movw r3, #1
	str r3, [r4, #4]
	movw r3, #2
	str r3, [r4, #8]
	movw r3, #3
	str r3, [r4, #12]
	movw r3, #4
	str r3, [r4, #16]
	str r4, [r2]
	ldr r2, =C$m
	str r2, [r1]
	ldr r1, [r0, #4]
	blx r1
	mov r9, r0
	str r9, [fp, #-40]
	ldr r10, [fp, #-44]
	mov r9, r10
	str r9, [fp, #-48]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #60
	add sp, sp, #28
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r10, [fp, #-48]
	sub r9, r10, #1
	str r9, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r4, r9
	ldr r10, [fp, #-40]
	ldr r9, [r10]
	str r9, [fp, #-60]
	movw r0, #0
	ldr r9, [fp, #-52]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-52]
	ldr r10, [fp, #-60]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-52]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r10, [fp, #-52]
	mov r9, r10
	str r9, [fp, #-48]
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
