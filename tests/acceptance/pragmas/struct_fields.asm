.struct mystruct:
a: .res 1
b: .res 2
c: .res 1
.endstruct
.word mystruct
.word a, b, c
