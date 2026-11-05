target triple = "dangcsa64"

@counter = global i32 0
@table = global [8 x i32] zeroinitializer

define i32 @shift_arith(i32 %a, i32 %b) {
  %r = ashr i32 %a, %b
  ret i32 %r
}

define i32 @shift_imm(i32 %a) {
  %x = shl i32 %a, 5
  %y = lshr i32 %x, 7
  %z = ashr i32 %y, 2
  ret i32 %z
}

define i32 @use_alloca(i32 %a) {
  %p = alloca i32
  store i32 %a, ptr %p
  %v = load i32, ptr %p
  %r = mul i32 %v, %v
  ret i32 %r
}

define i32 @use_global() {
  %v = load i32, ptr @counter
  %n = add i32 %v, 1
  store i32 %n, ptr @counter
  ret i32 %n
}

define i32 @index_table(i32 %i) {
  %p = getelementptr [8 x i32], ptr @table, i32 0, i32 %i
  %v = load i32, ptr %p
  ret i32 %v
}

define i32 @sum_loop(i32 %n) {
entry:
  br label %head
head:
  %i = phi i32 [ 0, %entry ], [ %i2, %body ]
  %acc = phi i32 [ 0, %entry ], [ %acc2, %body ]
  %c = icmp slt i32 %i, %n
  br i1 %c, label %body, label %done
body:
  %acc2 = add i32 %acc, %i
  %i2 = add i32 %i, 1
  br label %head
done:
  ret i32 %acc
}

define i32 @many_args(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f) {
  %x = add i32 %a, %b
  %y = add i32 %x, %c
  %z = add i32 %y, %d
  %w = add i32 %z, %e
  %r = add i32 %w, %f
  ret i32 %r
}

define i32 @callee_saved(i32 %a) {
  %x = call i32 @use_global()
  %y = call i32 @use_global()
  %s = add i32 %x, %y
  %r = add i32 %s, %a
  ret i32 %r
}
