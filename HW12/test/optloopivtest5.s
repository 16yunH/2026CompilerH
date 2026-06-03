.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10000, r0
	mov r10200, #10
	mov r10100, #0
	movw r118, #0
	cmp r10000, r118
	bgt main$L107
main$L108:
	mov r10103, #4
	mov r10104, r10103
main$L109:
	mov r0, r10104
	bl putint
	movw r117, #10
	mov r0, r117
	bl putch
	mov r0, r10104
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r10201, r10200
	mov r10101, r10100
main$L104:
	movw r119, #0
	cmp r10201, r119
	bgt main$L105
main$L106:
	mov r10104, r10101
	b main$L109
main$L105:
	movw r115, #3
	mul r11000, r115, r10201
	sub r10202, r10201, #1
	add r10102, r11000, #2
	mov r0, r10102
	bl putint
	movw r116, #32
	mov r0, r116
	bl putch
	mov r10201, r10202
	mov r10101, r10102
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
