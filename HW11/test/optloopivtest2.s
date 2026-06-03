.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
bl getint
mov t10000, r0
mov t10001, t10000
L102:
movw t117, #0
cmp t10001, t117
bgt L103
L104:
mov r0, t10001
bl putint
movw t116, #10
mov r0, t116
bl putch
movw t118, #2
mov r0, t118
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
movw t114, #4
mul t10900, t114, t10001
sub t10002, t10001, #2
add t10300, t10900, #2
mov r0, t10300
bl putint
movw t115, #32
mov r0, t115
bl putch
mov t10001, t10002
b L102

