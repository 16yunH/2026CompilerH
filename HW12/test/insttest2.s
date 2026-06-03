.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r130, #4
	mov r0, r130
	bl malloc
	mov r10300, r0
	mov r10100, #3
	movw r131, #20
	mov r0, r131
	bl malloc
	mov r10400, r0
	movw r132, #4
	str r132, [r10400]
	mov r12100, r10300
	mov r10000, r10300
	movw r133, #1
	str r133, [r10400, #4]
	movw r134, #2
	str r134, [r10400, #8]
	movw r135, #3
	str r135, [r10400, #12]
	movw r136, #4
	str r136, [r10400, #16]
	str r10400, [r12100]
	mov r10101, r10100
main$L102:
	movw r140, #0
	cmp r10101, r140
	bge main$L103
main$L104:
	movw r139, #10
	mov r0, r139
	bl putch
	movw r141, #2
	mov r0, r141
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10102, r10101, #1
	ldr r10500, [r10000]
	mov r11000, r10500
	ldr r10600, [r10500]
	movw r142, #0
	cmp r10102, r142
	bge main$L106
main$L105:
	movw r143, #65535
	movt r143, #65535
	mov r0, r143
	bl exit
main$L106:
	cmp r10102, r10600
	bge main$L105
main$L107:
	add r12400, r10102, #1
	movw r137, #4
	mul r12500, r12400, r137
	ldr r11100, [r11000, r12500]
	mov r0, r11100
	bl putint
	movw r138, #32
	mov r0, r138
	bl putch
	mov r10101, r10102
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
