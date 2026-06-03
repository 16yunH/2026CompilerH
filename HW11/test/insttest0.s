.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t112, #4
mov r0, t112
bl malloc
mov t10100, r0
ldr t113, =C^max
str t113, [t10100]
mov t10200, t10100
ldr t10400, [t10200]
movw t114, #100
movw t115, #200
mov r0, t10200
mov r1, t114
mov r2, t115
blx t10400
mov t10500, r0
mov r0, t10500
bl putint
movw t116, #1
mov r0, t116
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global C^max
C^max:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, r0
mov t101, r1
mov t102, r2
cmp t101, t102
bgt L102
L103:
mov r0, t102
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L102:
mov r0, t101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

