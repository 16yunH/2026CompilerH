.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r2, r0
	movw r0, #0
	cmp r1, r0
	bgt C$m$L102
C$m$L103:
	ldr r0, [r2, #4]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L102:
	ldr r0, [r2]
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
	movw r0, #12
	bl malloc
	mov r4, r0
	movw r0, #20
	bl malloc
	mov r1, r0
	movw r0, #4
	str r0, [r1]
	mov r0, r4
	add r10, r4, #4
	str r10, [fp, #-56]
	add r10, r4, #8
	str r10, [fp, #-60]
	movw r2, #1
	str r2, [r1, #4]
	movw r2, #2
	str r2, [r1, #8]
	movw r2, #3
	str r2, [r1, #12]
	movw r2, #4
	str r2, [r1, #16]
	str r1, [r0]
	movw r0, #24
	bl malloc
	mov r1, r0
	movw r0, #5
	str r0, [r1]
	add r0, r4, #8
	movw r2, #5
	str r2, [r1, #4]
	movw r2, #6
	str r2, [r1, #8]
	movw r2, #7
	str r2, [r1, #12]
	movw r2, #8
	str r2, [r1, #16]
	movw r2, #9
	str r2, [r1, #20]
	ldr r10, [fp, #-56]
	str r1, [r10]
	ldr r1, =C$m
	ldr r10, [fp, #-60]
	str r1, [r10]
	ldr r10, [r0]
	str r10, [fp, #-52]
	bl getint
	mov r1, r0
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	mov r0, r4
	ldr r9, [fp, #-52]
	blx r9
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-44]
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
	ldr r9, [fp, #-44]
	sub r10, r9, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r4, [r9]
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
	cmp r9, r4
	bge main$L105
main$L107:
	ldr r9, [fp, #-48]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-40]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-44]
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
