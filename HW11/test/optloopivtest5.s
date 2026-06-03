.global __$main__^main
__$main__^main:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
bl getint
mov t10000, r0
mov t10200, #10
mov t10100, #0
movw t118, #0
cmp t10000, t118
bgt L107
L108:
mov t10103, #4
mov t10104, t10103
L109:
mov r0, t10104
bl putint
movw t117, #10
mov r0, t117
bl putch
mov r0, t10104
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L107:
mov t10201, t10200
mov t10101, t10100
L104:
movw t119, #0
cmp t10201, t119
bgt L105
L106:
mov t10104, t10101
b L109
L105:
movw t115, #3
mul t11000, t115, t10201
sub t10202, t10201, #1
add t10102, t11000, #2
mov r0, t10102
bl putint
movw t116, #32
mov r0, t116
bl putch
mov t10201, t10202
mov t10101, t10102
b L104

