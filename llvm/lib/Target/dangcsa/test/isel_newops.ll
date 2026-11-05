; r0.5.0: 64-bit native multiply, MULH pair for i128, CLZ/CTZ, add-immediate folding.
; Expect: mul64 is one mul; mulh_signed is one mul hs; clz64/ctz64 are single
; instructions; addimm folds to add i with no quarter chain.

target triple = "dangcsa64"

define i64 @mul64(i64 %a, i64 %b) {
  %r = mul i64 %a, %b
  ret i64 %r
}

define i64 @mulh_signed(i64 %a, i64 %b) {
  %aw = sext i64 %a to i128
  %bw = sext i64 %b to i128
  %p = mul i128 %aw, %bw
  %hi = lshr i128 %p, 64
  %r = trunc i128 %hi to i64
  ret i64 %r
}

declare i64 @llvm.ctlz.i64(i64, i1)
declare i64 @llvm.cttz.i64(i64, i1)

define i64 @clz64(i64 %v) {
  %r = call i64 @llvm.ctlz.i64(i64 %v, i1 false)
  ret i64 %r
}

define i64 @ctz64(i64 %v) {
  %r = call i64 @llvm.cttz.i64(i64 %v, i1 false)
  ret i64 %r
}

define i64 @addimm(i64 %v) {
  %r = add i64 %v, 300
  ret i64 %r
}

define i64 @addimm_neg(i64 %v) {
  %r = add i64 %v, -12
  ret i64 %r
}

define i64 @bigimm() {
  ret i64 1311768467463790320
}
