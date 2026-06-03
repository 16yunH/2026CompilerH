.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10200, #0
	bl getint
	mov r10300, r0
	bl getint
	mov r10400, r0
	mov r10301, r10300
	mov r10201, r10200
main$L102:
	movw r120, #0
	cmp r10301, r120
	bgt main$L103
main$L104:
	movw r119, #10
	mov r0, r119
	bl putch
	mov r0, r10201
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10302, r10301, r10400
	movw r117, #8
	mul r11300, r117, r10302
	add r10202, r11300, #7
	mov r0, r10202
	bl putint
	movw r118, #32
	mov r0, r118
	bl putch
	mov r10301, r10302
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
