target triple = "dangcsa64"

define i32 @load_below(ptr %p) {
  %a = getelementptr i8, ptr %p, i64 -4
  %v = load i32, ptr %a
  ret i32 %v
}

define void @store_below(ptr %p, i64 %v) {
  %a = getelementptr i8, ptr %p, i64 -8
  store i64 %v, ptr %a
  ret void
}

define i64 @load_far_below(ptr %p) {
  %a = getelementptr i8, ptr %p, i64 -2147483648
  %v = load i64, ptr %a
  ret i64 %v
}

define i64 @load_out_of_range(ptr %p) {
  %a = getelementptr i8, ptr %p, i64 -2147483656
  %v = load i64, ptr %a
  ret i64 %v
}

define i8 @byte_below(ptr %p) {
  %a = getelementptr i8, ptr %p, i64 -1
  %v = load i8, ptr %a
  ret i8 %v
}
