.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r124, #20
	mov r0, r124
	bl malloc
	mov r10000, r0
	movw r125, #4
	str r125, [r10000]
	movw r126, #1
	str r126, [r10000, #4]
	movw r127, #2
	str r127, [r10000, #8]
	movw r128, #3
	str r128, [r10000, #12]
	movw r129, #4
	str r129, [r10000, #16]
	ldr r10300, [r10000]
	mov r10100, r10300
	mov r10101, r10100
main$L102:
	movw r133, #0
	cmp r10101, r133
	bgt main$L103
main$L104:
	movw r132, #10
	mov r0, r132
	bl putch
	movw r134, #2
	mov r0, r134
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10102, r10101, #1
	mov r10700, r10000
	ldr r10400, [r10000]
	movw r135, #0
	cmp r10102, r135
	bge main$L106
main$L105:
	movw r136, #65535
	movt r136, #65535
	mov r0, r136
	bl exit
main$L106:
	cmp r10102, r10400
	bge main$L105
main$L107:
	add r11800, r10102, #1
	movw r130, #4
	mul r11900, r11800, r130
	ldr r10800, [r10700, r11900]
	mov r0, r10800
	bl putint
	movw r131, #32
	mov r0, r131
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
