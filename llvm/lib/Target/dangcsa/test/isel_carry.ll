target triple = "dangcsa64"

define i64 @add64(i64 %a, i64 %b) {
  %r = add i64 %a, %b
  ret i64 %r
}

define i64 @sub64(i64 %a, i64 %b) {
  %r = sub i64 %a, %b
  ret i64 %r
}

define i64 @neg64(i64 %a) {
  %r = sub i64 0, %a
  ret i64 %r
}

define i64 @addconst64(i64 %a) {
  %r = add i64 %a, 4294967296
  ret i64 %r
}

define i64 @absv64(i64 %a) {
  %c = icmp slt i64 %a, 0
  %n = sub i64 0, %a
  %r = select i1 %c, i64 %n, i64 %a
  ret i64 %r
}
