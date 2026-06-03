.global __$main__^main
__$main__^main:
L113:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t171, #4
mov r0, t171
bl malloc
mov t10400, r0
mov t10200, #0
mov t10000, #0
ldr t172, =fib^f
str t172, [t10400]
mov t10201, t10400
movw t173, #69
mov r0, t173
bl putch
movw t174, #110
mov r0, t174
bl putch
movw t175, #116
mov r0, t175
bl putch
movw t176, #101
mov r0, t176
bl putch
movw t177, #114
mov r0, t177
bl putch
movw t178, #32
mov r0, t178
bl putch
movw t179, #116
mov r0, t179
bl putch
movw t180, #104
mov r0, t180
bl putch
movw t181, #101
mov r0, t181
bl putch
movw t182, #32
mov r0, t182
bl putch
movw t183, #110
mov r0, t183
bl putch
movw t184, #117
mov r0, t184
bl putch
movw t185, #109
mov r0, t185
bl putch
movw t186, #98
mov r0, t186
bl putch
movw t187, #101
mov r0, t187
bl putch
movw t188, #114
mov r0, t188
bl putch
movw t189, #32
mov r0, t189
bl putch
movw t190, #111
mov r0, t190
bl putch
movw t191, #102
mov r0, t191
bl putch
movw t192, #32
mov r0, t192
bl putch
movw t193, #116
mov r0, t193
bl putch
movw t194, #101
mov r0, t194
bl putch
movw t195, #114
mov r0, t195
bl putch
movw t196, #109
mov r0, t196
bl putch
movw t197, #58
mov r0, t197
bl putch
bl getint
mov t10100, r0
movw t202, #0
cmp t10100, t202
blt L105
L104:
movw t203, #47
cmp t10100, t203
bgt L105
L106:
L107:
mov t10001, t10000
L110:
cmp t10001, t10100
blt L111
L112:
movw t201, #10
mov r0, t201
bl putch
movw t204, #0
mov r0, t204
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L111:
ldr t13200, [t10201]
add t10002, t10001, #1
mov r0, t10201
mov r1, t10001
blx t13200
mov t13300, r0
mov r0, t13300
bl putint
movw t200, #32
mov r0, t200
bl putch
mov t10001, t10002
b L110
L105:
movw t198, #0
movw t199, #1
sub t16500, t198, t199
mov r0, t16500
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global fib^f
fib^f:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, r0
mov t101, r1
movw t119, #0
cmp t101, t119
beq L105
L104:
movw t120, #1
cmp t101, t120
beq L105
L106:
ldr t10300, [t100]
mov t10500, t100
sub t10400, t101, #1
mov t11600, t100
mov t11000, t100
sub t10900, t101, #2
mov t10600, t10300
mov r0, t10500
mov r1, t10400
blx t10600
mov t10700, r0
ldr t10800, [t11600]
mov t11300, t10700
mov t11100, t10800
mov r0, t11000
mov r1, t10900
blx t11100
mov t11200, r0
add t11800, t11300, t11200
mov r0, t11800
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L105:
mov r0, t101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

