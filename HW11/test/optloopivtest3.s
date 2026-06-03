.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10200, #0
bl getint
mov t10300, r0
bl getint
mov t10400, r0
mov t10301, t10300
mov t10201, t10200
L102:
movw t120, #0
cmp t10301, t120
bgt L103
L104:
movw t119, #10
mov r0, t119
bl putch
mov r0, t10201
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10302, t10301, t10400
movw t117, #8
mul t11300, t117, t10302
add t10202, t11300, #7
mov r0, t10202
bl putint
movw t118, #32
mov r0, t118
bl putch
mov t10301, t10302
mov t10201, t10202
b L102

