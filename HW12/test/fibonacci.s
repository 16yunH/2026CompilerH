.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r171, #4
	mov r0, r171
	bl malloc
	mov r10400, r0
	mov r10200, #0
	mov r10000, #0
	ldr r172, =fib$f
	str r172, [r10400]
	mov r10201, r10400
	movw r173, #69
	mov r0, r173
	bl putch
	movw r174, #110
	mov r0, r174
	bl putch
	movw r175, #116
	mov r0, r175
	bl putch
	movw r176, #101
	mov r0, r176
	bl putch
	movw r177, #114
	mov r0, r177
	bl putch
	movw r178, #32
	mov r0, r178
	bl putch
	movw r179, #116
	mov r0, r179
	bl putch
	movw r180, #104
	mov r0, r180
	bl putch
	movw r181, #101
	mov r0, r181
	bl putch
	movw r182, #32
	mov r0, r182
	bl putch
	movw r183, #110
	mov r0, r183
	bl putch
	movw r184, #117
	mov r0, r184
	bl putch
	movw r185, #109
	mov r0, r185
	bl putch
	movw r186, #98
	mov r0, r186
	bl putch
	movw r187, #101
	mov r0, r187
	bl putch
	movw r188, #114
	mov r0, r188
	bl putch
	movw r189, #32
	mov r0, r189
	bl putch
	movw r190, #111
	mov r0, r190
	bl putch
	movw r191, #102
	mov r0, r191
	bl putch
	movw r192, #32
	mov r0, r192
	bl putch
	movw r193, #116
	mov r0, r193
	bl putch
	movw r194, #101
	mov r0, r194
	bl putch
	movw r195, #114
	mov r0, r195
	bl putch
	movw r196, #109
	mov r0, r196
	bl putch
	movw r197, #58
	mov r0, r197
	bl putch
	bl getint
	mov r10100, r0
	movw r202, #0
	cmp r10100, r202
	blt main$L105
main$L104:
	movw r203, #47
	cmp r10100, r203
	bgt main$L105
main$L106:
main$L107:
	mov r10001, r10000
main$L110:
	cmp r10001, r10100
	blt main$L111
main$L112:
	movw r201, #10
	mov r0, r201
	bl putch
	movw r204, #0
	mov r0, r204
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L111:
	ldr r13200, [r10201]
	add r10002, r10001, #1
	mov r0, r10201
	mov r1, r10001
	blx r13200
	mov r13300, r0
	mov r0, r13300
	bl putint
	movw r200, #32
	mov r0, r200
	bl putch
	mov r10001, r10002
	b main$L110
main$L105:
	movw r198, #0
	movw r199, #1
	sub r16500, r198, r199
	mov r0, r16500
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global fib$f
.section .text
.arm
fib$f:
fib$f$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	movw r119, #0
	cmp r101, r119
	beq fib$f$L105
fib$f$L104:
	movw r120, #1
	cmp r101, r120
	beq fib$f$L105
fib$f$L106:
	ldr r10300, [r100]
	mov r10500, r100
	sub r10400, r101, #1
	mov r11600, r100
	mov r11000, r100
	sub r10900, r101, #2
	mov r10600, r10300
	mov r0, r10500
	mov r1, r10400
	blx r10600
	mov r10700, r0
	ldr r10800, [r11600]
	mov r11300, r10700
	mov r11100, r10800
	mov r0, r11000
	mov r1, r10900
	blx r11100
	mov r11200, r0
	add r11800, r11300, r11200
	mov r0, r11800
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
fib$f$L105:
	mov r0, r101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
