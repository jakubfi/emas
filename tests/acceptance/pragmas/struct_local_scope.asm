.struct point:
.x: .res 1
.y: .res 1
.endstruct
.struct rect:
.x: .res 1
.y: .res 1
.w: .res 1
.h: .res 1
.endstruct
.word point, rect
.word point.x, point.y
.word rect.x, rect.y, rect.w, rect.h
