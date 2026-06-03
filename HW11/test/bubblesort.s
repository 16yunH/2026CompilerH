.global b1^bubbleSort
b1^bubbleSort:
L133:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t101, r0
mov t102, r1
mov t103, r2
mov t10000, #0
movw t172, #1
cmp t103, t172
ble L102
L103:
L104:
mov t10001, t10000
L107:
sub t13300, t103, #1
cmp t10001, t13300
blt L108
L109:
ldr t12700, [t101, #4]
mov t13000, t101
mov t12900, t102
sub t12800, t103, #1
mov t13100, t12700
mov r0, t13000
mov r1, t12900
mov r2, t12800
blx t13100
mov t13200, r0
mov r0, t13200
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L108:
mov t11500, t102
ldr t10500, [t102]
movw t173, #0
cmp t10001, t173
bge L111
L110:
movw t174, #65535
movt t174, #65535
mov r0, t174
bl exit
L111:
cmp t10001, t10500
bge L110
L112:
add t13500, t10001, #1
add t10600, t10001, #1
mov t11700, t102
movw t166, #4
mul t13600, t13500, t166
ldr t11800, [t11500, t13600]
ldr t10700, [t102]
movw t175, #0
cmp t10600, t175
bge L114
L113:
movw t176, #65535
movt t176, #65535
mov r0, t176
bl exit
L114:
cmp t10600, t10700
bge L113
L115:
add t13900, t10600, #1
movw t167, #4
mul t14000, t13900, t167
ldr t14200, [t11700, t14000]
cmp t11800, t14200
bgt L130
L131:
L132:
add t10002, t10001, #1
mov t10001, t10002
b L107
L130:
mov t12000, t102
ldr t10800, [t102]
movw t177, #0
cmp t10001, t177
bge L119
L118:
movw t178, #65535
movt t178, #65535
mov r0, t178
bl exit
L119:
cmp t10001, t10800
bge L118
L120:
mov t14400, t101
add t14500, t10001, #1
mov t12200, t102
movw t168, #4
mul t14600, t14500, t168
ldr t14800, [t12000, t14600]
str t14800, [t14400]
ldr t10900, [t102]
movw t179, #0
cmp t10001, t179
bge L122
L121:
movw t180, #65535
movt t180, #65535
mov r0, t180
bl exit
L122:
cmp t10001, t10900
bge L121
L123:
add t11000, t10001, #1
mov t12400, t102
ldr t11100, [t102]
movw t181, #0
cmp t11000, t181
bge L125
L124:
movw t182, #65535
movt t182, #65535
mov r0, t182
bl exit
L125:
cmp t11000, t11100
bge L124
L126:
add t15100, t10001, #1
add t15400, t11000, #1
add t11200, t10001, #1
mov t12600, t102
movw t169, #4
mul t15200, t15100, t169
movw t170, #4
mul t15500, t15400, t170
add t15300, t12200, t15200
ldr t15700, [t12400, t15500]
str t15700, [t15300]
ldr t11300, [t102]
movw t183, #0
cmp t11200, t183
bge L128
L127:
movw t184, #65535
movt t184, #65535
mov r0, t184
bl exit
L128:
cmp t11200, t11300
bge L127
L129:
add t15900, t11200, #1
ldr t16300, [t101]
movw t171, #4
mul t16000, t15900, t171
str t16300, [t12600, t16000]
b L132
L102:
movw t185, #0
mov r0, t185
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t141, #32
mov r0, t141
bl malloc
mov t10000, r0
mov t10100, #0
mov t10200, #0
movw t142, #7
str t142, [t10000]
movw t143, #6
str t143, [t10000, #4]
movw t144, #3
str t144, [t10000, #8]
movw t145, #0
str t145, [t10000, #12]
movw t146, #5
str t146, [t10000, #16]
movw t147, #9
str t147, [t10000, #20]
movw t148, #1
str t148, [t10000, #24]
movw t149, #2
str t149, [t10000, #28]
movw t150, #8
mov r0, t150
bl malloc
mov t10400, r0
mov t11100, t10000
ldr t151, =b1^bubbleSort
str t151, [t10400, #4]
mov t10101, t10400
ldr t11000, [t10101, #4]
mov t11200, t10101
ldr t10500, [t10000]
mov t11300, t11000
mov r0, t11200
mov r1, t11100
mov r2, t10500
blx t11300
mov t10201, t10200
L102:
mov t11500, t10201
ldr t10600, [t10000]
cmp t11500, t10600
blt L103
L104:
movw t154, #10
mov r0, t154
bl putch
movw t155, #0
mov r0, t155
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
mov t11700, t10000
ldr t10700, [t10000]
movw t156, #0
cmp t10201, t156
bge L106
L105:
movw t157, #65535
movt t157, #65535
mov r0, t157
bl exit
L106:
cmp t10201, t10700
bge L105
L107:
add t13500, t10201, #1
add t10202, t10201, #1
movw t152, #4
mul t13600, t13500, t152
ldr t11800, [t11700, t13600]
mov r0, t11800
bl putint
movw t153, #32
mov r0, t153
bl putch
mov t10201, t10202
b L102

