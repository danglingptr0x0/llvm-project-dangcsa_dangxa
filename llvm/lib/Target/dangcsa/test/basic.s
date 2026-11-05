; RUN: llvm-mc -triple=dangcsa64 -show-encoding < %s | FileCheck %s

; CHECK: cpy dr1, dr0
cpy dr1, dr0

; CHECK: cpy i dr0, 305419896
cpy i dr0, 0x12345678

; CHECK: cpy q3 i dr0, 0x9000
cpy q3 i dr0, 0x9000

; CHECK: cpy q0 dr0, dr1
cpy q0 dr0, dr1

; CHECK: add dr2, dr3
add dr2, dr3

; CHECK: add i dr2, 300
add i dr2, 300

; CHECK: add c dr2, dr3
add c dr2, dr3

; CHECK: sub dr4, dr5
sub dr4, dr5

; CHECK: sub c i dr4, -12
sub c i dr4, -12

; CHECK: and dr1, dr2
and dr1, dr2

; CHECK: or dr3, dr4
or dr3, dr4

; CHECK: xor dr0, dr1
xor dr0, dr1

; CHECK: mul dr6, dr7
mul dr6, dr7

; CHECK: mul hu dr6, dr7
mul hu dr6, dr7

; CHECK: mul hs dr6, dr7
mul hs dr6, dr7

; CHECK: div u dr10, dr11
div u dr10, dr11

; CHECK: div ur dr10, dr11
div ur dr10, dr11

; CHECK: div s dr12, dr13
div s dr12, dr13

; CHECK: div sr dr12, dr13
div sr dr12, dr13

; CHECK: clz dr0, dr1
clz dr0, dr1

; CHECK: clz t dr0, dr1
clz t dr0, dr1

; CHECK: sh l i dr2, 4
sh l i dr2, 4

; CHECK: sh r i dr3, 4
sh r i dr3, 4

; CHECK: sh ra i dr3, 63
sh ra i dr3, 63

; CHECK: sh l dr2, dr3
sh l dr2, dr3

; CHECK: ld q dr0, [dr1 + 0]
ld q dr0, [dr1]

; CHECK: ld bs dr0, [dr1 + 8]
ld bs dr0, [dr1 + 8]

; CHECK: ld wu dr0, [dr1 + -8]
ld wu dr0, [dr1 + -8]

; CHECK: ld ds dr0, [sp + 16]
ld ds dr0, [sp + 16]

; CHECK: st q [dr1 + 0], dr0
st q [dr1], dr0

; CHECK: st b [dr1 + 3], dr0
st b [dr1 + 3], dr0

; CHECK: cpy tdr0, loc
cpy tdr0, loc

; CHECK: jmp dr0
jmp dr0

; CHECK: jmp ne dr0, dr1
jmp ne dr0, dr1

; CHECK: setcc geu dr2, dr3
setcc geu dr2, dr3

; CHECK: call dr0
call dr0

; CHECK: ret
ret
