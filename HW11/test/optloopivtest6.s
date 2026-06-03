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
mov t10201, t10200
mov t10101, t10100
L102:
movw t118, #0
cmp t10201, t118
bgt L103
L104:
mov r0, t10101
bl putint
movw t117, #10
mov r0, t117
bl putch
mov r0, t10101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
movw t119, #0
cmp t10000, t119
bgt L107
L108:
mov t10103, #4
mov t10104, t10103
L109:
sub t10202, t10201, #1
mov t10201, t10202
mov t10101, t10104
b L102
L107:
movw t115, #3
mul t11000, t115, t10201
add t10102, t11000, #2
mov r0, t10102
bl putint
movw t116, #32
mov r0, t116
bl putch
mov t10104, t10102
b L109

