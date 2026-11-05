target triple = "dangcsa64"

define i32 @pick(i32 %sel, i32 %acc) {
entry:
  switch i32 %sel, label %def [
    i32 0, label %c0
    i32 1, label %c1
    i32 2, label %c2
    i32 3, label %c3
    i32 4, label %c4
    i32 5, label %c5
    i32 6, label %c6
    i32 7, label %c7
  ]

c0:
  %r0 = add i32 %acc, 10
  br label %out

c1:
  %r1 = sub i32 %acc, 20
  br label %out

c2:
  %r2 = mul i32 %acc, 3
  br label %out

c3:
  %r3 = xor i32 %acc, 255
  br label %out

c4:
  %r4 = shl i32 %acc, 4
  br label %out

c5:
  %r5 = lshr i32 %acc, 2
  br label %out

c6:
  %r6 = and i32 %acc, 4095
  br label %out

c7:
  %r7 = or i32 %acc, 16
  br label %out

def:
  br label %out

out:
  %v = phi i32 [ %r0, %c0 ], [ %r1, %c1 ], [ %r2, %c2 ], [ %r3, %c3 ], [ %r4, %c4 ], [ %r5, %c5 ], [ %r6, %c6 ], [ %r7, %c7 ], [ 0, %def ]
  ret i32 %v
}
