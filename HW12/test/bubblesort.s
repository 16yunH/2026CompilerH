.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	mov r102, r1
	mov r103, r2
	mov r10000, #0
	movw r172, #1
	cmp r103, r172
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	mov r10001, r10000
b1$bubbleSort$L107:
	sub r13300, r103, #1
	cmp r10001, r13300
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	ldr r12700, [r101, #4]
	mov r13000, r101
	mov r12900, r102
	sub r12800, r103, #1
	mov r13100, r12700
	mov r0, r13000
	mov r1, r12900
	mov r2, r12800
	blx r13100
	mov r13200, r0
	mov r0, r13200
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	mov r11500, r102
	ldr r10500, [r102]
	movw r173, #0
	cmp r10001, r173
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	movw r174, #65535
	movt r174, #65535
	mov r0, r174
	bl exit
b1$bubbleSort$L111:
	cmp r10001, r10500
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	add r13500, r10001, #1
	add r10600, r10001, #1
	mov r11700, r102
	movw r166, #4
	mul r13600, r13500, r166
	ldr r11800, [r11500, r13600]
	ldr r10700, [r102]
	movw r175, #0
	cmp r10600, r175
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r176, #65535
	movt r176, #65535
	mov r0, r176
	bl exit
b1$bubbleSort$L114:
	cmp r10600, r10700
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	add r13900, r10600, #1
	movw r167, #4
	mul r14000, r13900, r167
	ldr r14200, [r11700, r14000]
	cmp r11800, r14200
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
b1$bubbleSort$L132:
	add r10002, r10001, #1
	mov r10001, r10002
	b b1$bubbleSort$L107
b1$bubbleSort$L130:
	mov r12000, r102
	ldr r10800, [r102]
	movw r177, #0
	cmp r10001, r177
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	movw r178, #65535
	movt r178, #65535
	mov r0, r178
	bl exit
b1$bubbleSort$L119:
	cmp r10001, r10800
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	mov r14400, r101
	add r14500, r10001, #1
	mov r12200, r102
	movw r168, #4
	mul r14600, r14500, r168
	ldr r14800, [r12000, r14600]
	str r14800, [r14400]
	ldr r10900, [r102]
	movw r179, #0
	cmp r10001, r179
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	movw r180, #65535
	movt r180, #65535
	mov r0, r180
	bl exit
b1$bubbleSort$L122:
	cmp r10001, r10900
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	add r11000, r10001, #1
	mov r12400, r102
	ldr r11100, [r102]
	movw r181, #0
	cmp r11000, r181
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r182, #65535
	movt r182, #65535
	mov r0, r182
	bl exit
b1$bubbleSort$L125:
	cmp r11000, r11100
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	add r15100, r10001, #1
	add r15400, r11000, #1
	add r11200, r10001, #1
	mov r12600, r102
	movw r169, #4
	mul r15200, r15100, r169
	movw r170, #4
	mul r15500, r15400, r170
	add r15300, r12200, r15200
	ldr r15700, [r12400, r15500]
	str r15700, [r15300]
	ldr r11300, [r102]
	movw r183, #0
	cmp r11200, r183
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r184, #65535
	movt r184, #65535
	mov r0, r184
	bl exit
b1$bubbleSort$L128:
	cmp r11200, r11300
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	add r15900, r11200, #1
	ldr r16300, [r101]
	movw r171, #4
	mul r16000, r15900, r171
	str r16300, [r12600, r16000]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r185, #0
	mov r0, r185
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
	sub sp, sp, #4
	add fp, sp, #36
	movw r141, #32
	mov r0, r141
	bl malloc
	mov r10000, r0
	mov r10100, #0
	mov r10200, #0
	movw r142, #7
	str r142, [r10000]
	movw r143, #6
	str r143, [r10000, #4]
	movw r144, #3
	str r144, [r10000, #8]
	movw r145, #0
	str r145, [r10000, #12]
	movw r146, #5
	str r146, [r10000, #16]
	movw r147, #9
	str r147, [r10000, #20]
	movw r148, #1
	str r148, [r10000, #24]
	movw r149, #2
	str r149, [r10000, #28]
	movw r150, #8
	mov r0, r150
	bl malloc
	mov r10400, r0
	mov r11100, r10000
	ldr r151, =b1$bubbleSort
	str r151, [r10400, #4]
	mov r10101, r10400
	ldr r11000, [r10101, #4]
	mov r11200, r10101
	ldr r10500, [r10000]
	mov r11300, r11000
	mov r0, r11200
	mov r1, r11100
	mov r2, r10500
	blx r11300
	mov r10201, r10200
main$L102:
	mov r11500, r10201
	ldr r10600, [r10000]
	cmp r11500, r10600
	blt main$L103
main$L104:
	movw r154, #10
	mov r0, r154
	bl putch
	movw r155, #0
	mov r0, r155
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r11700, r10000
	ldr r10700, [r10000]
	movw r156, #0
	cmp r10201, r156
	bge main$L106
main$L105:
	movw r157, #65535
	movt r157, #65535
	mov r0, r157
	bl exit
main$L106:
	cmp r10201, r10700
	bge main$L105
main$L107:
	add r13500, r10201, #1
	add r10202, r10201, #1
	movw r152, #4
	mul r13600, r13500, r152
	ldr r11800, [r11700, r13600]
	mov r0, r11800
	bl putint
	movw r153, #32
	mov r0, r153
	bl putch
	mov r10201, r10202
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
