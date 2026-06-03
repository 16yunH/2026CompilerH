.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	movw r107, #0
	cmp r101, r107
	bgt C$m$L102
C$m$L103:
	ldr r10600, [r100, #4]
	mov r0, r10600
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L102:
	ldr r10400, [r100]
	mov r0, r10400
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
	movw r148, #12
	mov r0, r148
	bl malloc
	mov r10300, r0
	movw r149, #20
	mov r0, r149
	bl malloc
	mov r10400, r0
	movw r150, #4
	str r150, [r10400]
	mov r12900, r10300
	add r13600, r10300, #4
	add r13700, r10300, #8
	mov r10600, r10300
	movw r151, #1
	str r151, [r10400, #4]
	movw r152, #2
	str r152, [r10400, #8]
	movw r153, #3
	str r153, [r10400, #12]
	movw r154, #4
	str r154, [r10400, #16]
	str r10400, [r12900]
	movw r155, #24
	mov r0, r155
	bl malloc
	mov r10500, r0
	movw r156, #5
	str r156, [r10500]
	add r13800, r10600, #8
	mov r11400, r10600
	movw r157, #5
	str r157, [r10500, #4]
	movw r158, #6
	str r158, [r10500, #8]
	movw r159, #7
	str r159, [r10500, #12]
	movw r160, #8
	str r160, [r10500, #16]
	movw r161, #9
	str r161, [r10500, #20]
	str r10500, [r13600]
	ldr r162, =C$m
	str r162, [r13700]
	ldr r11200, [r13800]
	bl getint
	mov r11300, r0
	mov r11500, r11200
	mov r0, r11400
	mov r1, r11300
	blx r11500
	mov r11600, r0
	mov r10000, r11600
	ldr r10700, [r10000]
	mov r10100, r10700
	mov r10101, r10100
main$L102:
	movw r166, #0
	cmp r10101, r166
	bge main$L103
main$L104:
	movw r165, #10
	mov r0, r165
	bl putch
	movw r167, #2
	mov r0, r167
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10102, r10101, #1
	mov r11800, r10000
	ldr r10800, [r10000]
	movw r168, #0
	cmp r10102, r168
	bge main$L106
main$L105:
	movw r169, #65535
	movt r169, #65535
	mov r0, r169
	bl exit
main$L106:
	cmp r10102, r10800
	bge main$L105
main$L107:
	add r14200, r10102, #1
	movw r163, #4
	mul r14300, r14200, r163
	ldr r11900, [r11800, r14300]
	mov r0, r11900
	bl putint
	movw r164, #32
	mov r0, r164
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
