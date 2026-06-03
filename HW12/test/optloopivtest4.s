.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10100, #10
	mov r10101, r10100
main$L102:
	movw r115, #0
	cmp r10101, r115
	bgt main$L103
main$L104:
	mov r0, r10101
	bl putint
	movw r114, #10
	mov r0, r114
	bl putch
	mov r0, r10101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10102, r10101, #1
	movw r112, #4
	mul r10700, r112, r10102
	sub r10000, r10700, #2
	mov r0, r10000
	bl putint
	movw r113, #32
	mov r0, r113
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
