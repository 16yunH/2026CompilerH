.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #4
	bl malloc
	ldr r1, =C$max
	str r1, [r0]
	ldr r10, [r0]
	str r10, [fp, #-40]
	movw r1, #100
	movw r2, #200
	ldr r9, [fp, #-40]
	blx r9
	bl putint
	movw r0, #1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global C$max
.section .text
.arm
C$max:
C$max$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r2
	cmp r1, r0
	bgt C$max$L102
C$max$L103:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$max$L102:
	mov r0, r1
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
