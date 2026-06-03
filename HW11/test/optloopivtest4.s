.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10100, #10
mov t10101, t10100
L102:
movw t115, #0
cmp t10101, t115
bgt L103
L104:
mov r0, t10101
bl putint
movw t114, #10
mov r0, t114
bl putch
mov r0, t10101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
movw t112, #4
mul t10700, t112, t10102
sub t10000, t10700, #2
mov r0, t10000
bl putint
movw t113, #32
mov r0, t113
bl putch
mov t10101, t10102
b L102

