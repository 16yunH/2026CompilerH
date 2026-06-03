.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	mov r10, r0
	str r10, [fp, #-40]
	mov r10, r1
	str r10, [fp, #-44]
	mov r10, r2
	str r10, [fp, #-48]
	mov r10, #0
	str r10, [fp, #-52]
	movw r0, #1
	ldr r9, [fp, #-48]
	cmp r9, r0
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
b1$bubbleSort$L107:
	ldr r9, [fp, #-48]
	sub r0, r9, #1
	ldr r9, [fp, #-52]
	cmp r9, r0
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	ldr r9, [fp, #-40]
	ldr r3, [r9, #4]
	ldr r9, [fp, #-40]
	mov r0, r9
	ldr r9, [fp, #-44]
	mov r1, r9
	ldr r9, [fp, #-48]
	sub r2, r9, #1
	blx r3
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	ldr r9, [fp, #-44]
	mov r4, r9
	ldr r9, [fp, #-44]
	ldr r5, [r9]
	movw r0, #0
	ldr r9, [fp, #-52]
	cmp r9, r0
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L111:
	ldr r9, [fp, #-52]
	cmp r9, r5
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	ldr r9, [fp, #-52]
	add r0, r9, #1
	ldr r9, [fp, #-52]
	add r6, r9, #1
	ldr r9, [fp, #-44]
	mov r7, r9
	movw r1, #4
	mul r0, r0, r1
	ldr r4, [r4, r0]
	ldr r9, [fp, #-44]
	ldr r5, [r9]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L114:
	cmp r6, r5
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r7, r0]
	cmp r4, r0
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
b1$bubbleSort$L132:
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-52]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	b b1$bubbleSort$L107
b1$bubbleSort$L130:
	ldr r9, [fp, #-44]
	mov r4, r9
	ldr r9, [fp, #-44]
	ldr r5, [r9]
	movw r0, #0
	ldr r9, [fp, #-52]
	cmp r9, r0
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L119:
	ldr r9, [fp, #-52]
	cmp r9, r5
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-52]
	add r0, r9, #1
	ldr r9, [fp, #-44]
	mov r7, r9
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	ldr r10, [fp, #-40]
	str r0, [r10]
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-52]
	cmp r9, r0
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L122:
	ldr r9, [fp, #-52]
	cmp r9, r4
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	ldr r9, [fp, #-52]
	add r6, r9, #1
	ldr r9, [fp, #-44]
	mov r4, r9
	ldr r9, [fp, #-44]
	ldr r5, [r9]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L125:
	cmp r6, r5
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	ldr r9, [fp, #-52]
	add r1, r9, #1
	add r0, r6, #1
	ldr r9, [fp, #-52]
	add r5, r9, #1
	ldr r9, [fp, #-44]
	mov r6, r9
	movw r2, #4
	mul r1, r1, r2
	movw r2, #4
	mul r0, r0, r2
	add r1, r7, r1
	ldr r0, [r4, r0]
	str r0, [r1]
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	cmp r5, r0
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L128:
	cmp r5, r4
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	add r1, r5, #1
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	movw r2, #4
	mul r1, r1, r2
	str r0, [r6, r1]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r0, #0
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	movw r0, #32
	bl malloc
	mov r10, r0
	str r10, [fp, #-40]
	mov r0, #0
	mov r10, #0
	str r10, [fp, #-44]
	movw r0, #7
	ldr r10, [fp, #-40]
	str r0, [r10]
	movw r0, #6
	ldr r10, [fp, #-40]
	str r0, [r10, #4]
	movw r0, #3
	ldr r10, [fp, #-40]
	str r0, [r10, #8]
	movw r0, #0
	ldr r10, [fp, #-40]
	str r0, [r10, #12]
	movw r0, #5
	ldr r10, [fp, #-40]
	str r0, [r10, #16]
	movw r0, #9
	ldr r10, [fp, #-40]
	str r0, [r10, #20]
	movw r0, #1
	ldr r10, [fp, #-40]
	str r0, [r10, #24]
	movw r0, #2
	ldr r10, [fp, #-40]
	str r0, [r10, #28]
	movw r0, #8
	bl malloc
	ldr r9, [fp, #-40]
	mov r1, r9
	ldr r2, =b1$bubbleSort
	str r2, [r0, #4]
	ldr r3, [r0, #4]
	ldr r9, [fp, #-40]
	ldr r2, [r9]
	blx r3
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	ldr r9, [fp, #-44]
	cmp r9, r0
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-44]
	cmp r9, r4
	bge main$L105
main$L107:
	ldr r9, [fp, #-44]
	add r0, r9, #1
	ldr r9, [fp, #-44]
	add r10, r9, #1
	str r10, [fp, #-44]
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-40]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-44]
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
