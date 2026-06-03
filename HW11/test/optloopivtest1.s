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
movw t115, #0
cmp t10001, t115
bgt L103
L104:
movw t114, #10
mov r0, t114
bl putch
movw t116, #2
mov r0, t116
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10002, t10001, #1
movw t112, #4
mul t10800, t112, t10002
add t10300, t10800, #2
mov r0, t10300
bl putint
movw t113, #32
mov r0, t113
bl putch
mov t10001, t10002
b L102

