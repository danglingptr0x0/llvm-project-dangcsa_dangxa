target triple = "dangcsa64"

define i32 @add_two(i32 %a, i32 %b) {
  %r = add i32 %a, %b
  ret i32 %r
}

define i32 @sub_two(i32 %a, i32 %b) {
  %r = sub i32 %a, %b
  ret i32 %r
}

define i32 @bitops(i32 %a, i32 %b) {
  %x = and i32 %a, %b
  %y = or i32 %x, %b
  %z = xor i32 %y, %a
  ret i32 %z
}

define i32 @shifts(i32 %a, i32 %b) {
  %x = shl i32 %a, %b
  %y = lshr i32 %x, 3
  %z = ashr i32 %y, %b
  ret i32 %z
}

define i32 @muldiv(i32 %a, i32 %b) {
  %x = mul i32 %a, %b
  %y = sdiv i32 %x, %b
  %z = srem i32 %y, %b
  ret i32 %z
}

define i32 @load_store(ptr %p, i32 %v) {
  store i32 %v, ptr %p
  %r = load i32, ptr %p
  ret i32 %r
}

define i32 @sized(ptr %p) {
  %b = load i8, ptr %p
  %bx = sext i8 %b to i32
  %w = load i16, ptr %p
  %wx = zext i16 %w to i32
  %r = add i32 %bx, %wx
  ret i32 %r
}

define i32 @cmp_branch(i32 %a, i32 %b) {
entry:
  %c = icmp slt i32 %a, %b
  br i1 %c, label %lt, label %ge
lt:
  ret i32 1
ge:
  ret i32 0
}

define i32 @setcc(i32 %a, i32 %b) {
  %c = icmp eq i32 %a, %b
  %r = zext i1 %c to i32
  ret i32 %r
}

define i32 @call_it(i32 %a) {
  %r = call i32 @add_two(i32 %a, i32 7)
  ret i32 %r
}
