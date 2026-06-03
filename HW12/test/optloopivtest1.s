.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10000, r0
	mov r10001, r10000
main$L102:
	movw r115, #0
	cmp r10001, r115
	bgt main$L103
main$L104:
	movw r114, #10
	mov r0, r114
	bl putch
	movw r116, #2
	mov r0, r116
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10002, r10001, #1
	movw r112, #4
	mul r10800, r112, r10002
	add r10300, r10800, #2
	mov r0, r10300
	bl putint
	movw r113, #32
	mov r0, r113
	bl putch
	mov r10001, r10002
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
