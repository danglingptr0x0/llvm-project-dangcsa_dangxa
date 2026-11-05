target triple = "dangcsa64"

define i64 @udiv64(i64 %a, i64 %b) {
  %r = udiv i64 %a, %b
  ret i64 %r
}

define i64 @sdiv64(i64 %a, i64 %b) {
  %r = sdiv i64 %a, %b
  ret i64 %r
}

define i64 @shl64(i64 %a, i64 %b) {
  %r = shl i64 %a, %b
  ret i64 %r
}

define i64 @mul64(i64 %a, i64 %b) {
  %r = mul i64 %a, %b
  ret i64 %r
}

declare void @llvm.memcpy.p0.p0.i64(ptr, ptr, i64, i1)

define void @bigcopy(ptr %dst, ptr %src) {
  call void @llvm.memcpy.p0.p0.i64(ptr %dst, ptr %src, i64 4096, i1 false)
  ret void
}
