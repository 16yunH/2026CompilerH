.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #96
	add fp, sp, #128
	mov r10, r0
	str r10, [fp, #-40]
	mov r10, r1
	str r10, [fp, #-44]
	mov r10, r2
	str r10, [fp, #-48]
	mov r0, #0
	movw r1, #1
	ldr r9, [fp, #-48]
	cmp r9, r1
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	mov r10, r0
	str r10, [fp, #-52]
b1$bubbleSort$L107:
	ldr r9, [fp, #-48]
	sub r0, r9, #1
	ldr r9, [fp, #-52]
	cmp r9, r0
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	ldr r9, [fp, #-40]
	ldr r10, [r9, #4]
	str r10, [fp, #-120]
	ldr r9, [fp, #-40]
	mov r0, r9
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-48]
	sub r2, r9, #1
	ldr r9, [fp, #-120]
	mov r10, r9
	str r10, [fp, #-120]
	ldr r9, [fp, #-44]
	mov r1, r9
	ldr r9, [fp, #-120]
	blx r9
	sub sp, fp, #128
	add sp, sp, #96
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-92]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-56]
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
	ldr r10, [fp, #-56]
	cmp r9, r10
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	ldr r9, [fp, #-52]
	add r0, r9, #1
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-60]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-96]
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-92]
	ldr r10, [r9, r0]
	str r10, [fp, #-100]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-64]
	movw r0, #0
	ldr r9, [fp, #-60]
	cmp r9, r0
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L114:
	ldr r9, [fp, #-60]
	ldr r10, [fp, #-64]
	cmp r9, r10
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	ldr r9, [fp, #-60]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-96]
	ldr r0, [r9, r0]
	ldr r9, [fp, #-100]
	cmp r9, r0
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
	mov r10, r9
	str r10, [fp, #-104]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-68]
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
	ldr r10, [fp, #-68]
	cmp r9, r10
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-52]
	add r0, r9, #1
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-108]
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-104]
	ldr r0, [r9, r0]
	ldr r10, [fp, #-40]
	str r0, [r10]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-72]
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
	ldr r10, [fp, #-72]
	cmp r9, r10
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-76]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-112]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-80]
	movw r0, #0
	ldr r9, [fp, #-76]
	cmp r9, r0
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L125:
	ldr r9, [fp, #-76]
	ldr r10, [fp, #-80]
	cmp r9, r10
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	ldr r9, [fp, #-52]
	add r0, r9, #1
	ldr r9, [fp, #-76]
	add r10, r9, #1
	str r10, [fp, #-124]
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-84]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-116]
	movw r1, #4
	mul r1, r0, r1
	movw r0, #4
	ldr r9, [fp, #-124]
	mul r0, r9, r0
	ldr r9, [fp, #-108]
	add r1, r9, r1
	ldr r9, [fp, #-112]
	ldr r0, [r9, r0]
	str r0, [r1]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-88]
	movw r0, #0
	ldr r9, [fp, #-84]
	cmp r9, r0
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L128:
	ldr r9, [fp, #-84]
	ldr r10, [fp, #-88]
	cmp r9, r10
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	ldr r9, [fp, #-84]
	add r0, r9, #1
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-128]
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-128]
	ldr r10, [fp, #-116]
	str r9, [r10, r0]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r0, #0
	sub sp, fp, #128
	add sp, sp, #96
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
	mov r10, r9
	str r10, [fp, #-40]
	ldr r1, =b1$bubbleSort
	str r1, [r0, #4]
	ldr r10, [r0, #4]
	str r10, [fp, #-60]
	ldr r9, [fp, #-40]
	ldr r2, [r9]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-40]
	mov r1, r9
	ldr r9, [fp, #-60]
	blx r9
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
main$L102:
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	ldr r9, [fp, #-48]
	cmp r9, r0
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #60
	add sp, sp, #28
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-56]
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
	ldr r10, [fp, #-56]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-48]
	add r0, r9, #1
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-52]
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
