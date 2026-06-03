.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	ldr r10300, [r100]
	mov r0, r10300
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
	movw r135, #8
	mov r0, r135
	bl malloc
	mov r10400, r0
	mov r10200, #3
	movw r136, #20
	mov r0, r136
	bl malloc
	mov r10500, r0
	movw r137, #4
	str r137, [r10500]
	mov r12400, r10400
	add r12500, r10400, #4
	mov r10600, r10400
	movw r138, #1
	str r138, [r10500, #4]
	movw r139, #2
	str r139, [r10500, #8]
	movw r140, #3
	str r140, [r10500, #12]
	movw r141, #4
	str r141, [r10500, #16]
	str r10500, [r12400]
	ldr r142, =C$m
	str r142, [r12500]
	ldr r11000, [r10600, #4]
	mov r0, r10600
	blx r11000
	mov r11100, r0
	mov r10000, r11100
	mov r10201, r10200
main$L102:
	movw r146, #0
	cmp r10201, r146
	bge main$L103
main$L104:
	movw r145, #10
	mov r0, r145
	bl putch
	movw r147, #2
	mov r0, r147
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10202, r10201, #1
	mov r11300, r10000
	ldr r10700, [r10000]
	movw r148, #0
	cmp r10202, r148
	bge main$L106
main$L105:
	movw r149, #65535
	movt r149, #65535
	mov r0, r149
	bl exit
main$L106:
	cmp r10202, r10700
	bge main$L105
main$L107:
	add r12900, r10202, #1
	movw r143, #4
	mul r13000, r12900, r143
	ldr r11400, [r11300, r13000]
	mov r0, r11400
	bl putint
	movw r144, #32
	mov r0, r144
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
