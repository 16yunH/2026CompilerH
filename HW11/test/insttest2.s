.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t130, #4
mov r0, t130
bl malloc
mov t10300, r0
mov t10100, #3
movw t131, #20
mov r0, t131
bl malloc
mov t10400, r0
movw t132, #4
str t132, [t10400]
mov t12100, t10300
mov t10000, t10300
movw t133, #1
str t133, [t10400, #4]
movw t134, #2
str t134, [t10400, #8]
movw t135, #3
str t135, [t10400, #12]
movw t136, #4
str t136, [t10400, #16]
str t10400, [t12100]
mov t10101, t10100
L102:
movw t140, #0
cmp t10101, t140
bge L103
L104:
movw t139, #10
mov r0, t139
bl putch
movw t141, #2
mov r0, t141
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
ldr t10500, [t10000]
mov t11000, t10500
ldr t10600, [t10500]
movw t142, #0
cmp t10102, t142
bge L106
L105:
movw t143, #65535
movt t143, #65535
mov r0, t143
bl exit
L106:
cmp t10102, t10600
bge L105
L107:
add t12400, t10102, #1
movw t137, #4
mul t12500, t12400, t137
ldr t11100, [t11000, t12500]
mov r0, t11100
bl putint
movw t138, #32
mov r0, t138
bl putch
mov t10101, t10102
b L102

