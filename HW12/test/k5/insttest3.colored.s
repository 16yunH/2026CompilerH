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
	sub sp, sp, #24
	add fp, sp, #56
	movw r0, #8
	bl malloc
	mov r4, r0
	mov r10, #3
	str r10, [fp, #-44]
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-56]
	movw r0, #4
	ldr r10, [fp, #-56]
	str r0, [r10]
	mov r1, r4
	add r0, r4, #4
	mov r2, r4
	movw r3, #1
	ldr r10, [fp, #-56]
	str r3, [r10, #4]
	movw r3, #2
	ldr r10, [fp, #-56]
	str r3, [r10, #8]
	movw r3, #3
	ldr r10, [fp, #-56]
	str r3, [r10, #12]
	movw r3, #4
	ldr r10, [fp, #-56]
	str r3, [r10, #16]
	ldr r9, [fp, #-56]
	str r9, [r1]
	ldr r1, =C$m
	str r1, [r0]
	ldr r1, [r2, #4]
	mov r0, r2
	blx r1
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-48]
	sub r10, r9, #1
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r4, [r9]
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
	cmp r9, r4
	bge main$L105
main$L107:
	ldr r9, [fp, #-52]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-40]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-48]
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
