#include "dangcsa.h"
#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "dangcsa-float-expand"

namespace {

struct dangcsaFloatFmt {
  unsigned Width;
  unsigned FracBits;
  uint64_t ExpMax;
  int64_t Bias;
  const char *Tag;

  uint64_t fracMask() const { return (uint64_t(1) << FracBits) - 1; }
  uint64_t implicitBit() const { return uint64_t(1) << FracBits; }
  uint64_t signBit() const { return uint64_t(1) << (Width - 1); }
  uint64_t absMask() const { return signBit() - 1; }
  uint64_t infBits() const { return ExpMax << FracBits; }
  uint64_t quietBit() const { return uint64_t(1) << (FracBits - 1); }
  uint64_t qnanBits() const { return infBits() | quietBit(); }
  uint64_t oneBits() const { return uint64_t(Bias) << FracBits; }
  unsigned top() const { return FracBits + 3; }
};

const dangcsaFloatFmt dangcsaFmtF32 = {32, 23, 255, 127, "f32"};
const dangcsaFloatFmt dangcsaFmtF64 = {64, 52, 2047, 1023, "f64"};

enum class dangcsaRound { Trunc, Floor, Ceil, Away, Even };

struct dangcsaUnpacked {
  Value *Sign;
  Value *Exp;
  Value *Sig;
  Value *IsZero;
  Value *IsInf;
  Value *IsNaN;
};

class dangcsaFloatGen {
public:
  explicit dangcsaFloatGen(IRBuilder<> &B)
      : B(B), I64(B.getInt64Ty()), I128(B.getInt128Ty()) {}

  Value *c(uint64_t V) { return B.getInt64(V); }

  Value *cs(int64_t V) { return ConstantInt::getSigned(I64, V); }

  Value *c128(uint64_t V) { return ConstantInt::get(I128, V); }

  Value *ctlz(Value *V) {
    return B.CreateIntrinsic(Intrinsic::ctlz, {V->getType()},
                             {V, B.getFalse()});
  }

  Value *shrSticky(Value *M, Value *S) {
    Value *Big = B.CreateICmpUGE(S, c(64));
    Value *Amt = B.CreateSelect(Big, c(63), S);
    Value *Kept = B.CreateLShr(M, Amt);
    Value *Mask = B.CreateSub(B.CreateShl(c(1), Amt), c(1));
    Value *Lost = B.CreateZExt(B.CreateICmpNE(B.CreateAnd(M, Mask), c(0)), I64);
    Value *All = B.CreateZExt(B.CreateICmpNE(M, c(0)), I64);
    return B.CreateSelect(Big, All, B.CreateOr(Kept, Lost));
  }

  std::pair<Value *, Value *> normalize(Value *M, unsigned Top) {
    Value *Lead = B.CreateSub(c(63), ctlz(M));
    Value *Delta = B.CreateSub(Lead, c(Top));
    Value *Right = B.CreateICmpSGT(Delta, c(0));
    Value *RAmt = B.CreateSelect(Right, Delta, c(0));
    Value *LAmt = B.CreateSelect(Right, c(0), B.CreateNeg(Delta));
    LAmt = B.CreateSelect(B.CreateICmpUGT(LAmt, c(63)), c(63), LAmt);
    Value *Res =
        B.CreateSelect(Right, shrSticky(M, RAmt), B.CreateShl(M, LAmt));
    return {Res, Delta};
  }

  dangcsaUnpacked unpack(const dangcsaFloatFmt &F, Value *X) {
    Value *Exp = B.CreateAnd(B.CreateLShr(X, c(F.FracBits)), c(F.ExpMax));
    Value *Frac = B.CreateAnd(X, c(F.fracMask()));
    Value *ExpZero = B.CreateICmpEQ(Exp, c(0));
    Value *Sig =
        B.CreateSelect(ExpZero, Frac, B.CreateOr(Frac, c(F.implicitBit())));
    Value *E = B.CreateSelect(ExpZero, c(1), Exp);
    auto [M, Delta] = normalize(B.CreateShl(Sig, c(3)), F.top());
    Value *Abs = B.CreateAnd(X, c(F.absMask()));
    dangcsaUnpacked U;
    U.Sign = B.CreateAnd(B.CreateLShr(X, c(F.Width - 1)), c(1));
    U.Exp = B.CreateAdd(E, Delta);
    U.Sig = M;
    U.IsZero = B.CreateICmpEQ(Abs, c(0));
    U.IsInf = B.CreateICmpEQ(Abs, c(F.infBits()));
    U.IsNaN = B.CreateICmpUGT(Abs, c(F.infBits()));
    return U;
  }

  Value *signedZero(const dangcsaFloatFmt &F, Value *Sign) {
    return B.CreateShl(Sign, c(F.Width - 1));
  }

  Value *signedInf(const dangcsaFloatFmt &F, Value *Sign) {
    return B.CreateOr(signedZero(F, Sign), c(F.infBits()));
  }

  Value *pack(const dangcsaFloatFmt &F, Value *Sign, Value *E, Value *M) {
    Value *Sub = B.CreateICmpSLE(E, c(0));
    Value *Shift = B.CreateSelect(Sub, B.CreateSub(c(1), E), c(0));
    Value *M1 = shrSticky(M, Shift);
    Value *Low = B.CreateAnd(M1, c(7));
    Value *Q = B.CreateLShr(M1, c(3));
    Value *Odd = B.CreateICmpNE(B.CreateAnd(Q, c(1)), c(0));
    Value *Up = B.CreateOr(B.CreateICmpUGT(Low, c(4)),
                           B.CreateAnd(B.CreateICmpEQ(Low, c(4)), Odd));
    Value *R = B.CreateAdd(Q, B.CreateZExt(Up, I64));
    Value *Carry = B.CreateICmpUGE(R, c(F.implicitBit() << 1));
    Value *R2 = B.CreateSelect(Carry, B.CreateLShr(R, c(1)), R);
    Value *E2 = B.CreateSelect(Carry, B.CreateAdd(E, c(1)), E);
    Value *Norm = B.CreateOr(B.CreateShl(E2, c(F.FracBits)),
                             B.CreateAnd(R2, c(F.fracMask())));
    Norm =
        B.CreateSelect(B.CreateICmpSGE(E2, c(F.ExpMax)), c(F.infBits()), Norm);
    Value *Bits = B.CreateSelect(Sub, R, Norm);
    return B.CreateOr(Bits, signedZero(F, Sign));
  }

  Value *add(const dangcsaFloatFmt &F, Value *A, Value *Bv) {
    dangcsaUnpacked UA = unpack(F, A);
    dangcsaUnpacked UB = unpack(F, Bv);
    Value *AAbs = B.CreateAnd(A, c(F.absMask()));
    Value *BAbs = B.CreateAnd(Bv, c(F.absMask()));
    Value *Swap = B.CreateICmpUGT(BAbs, AAbs);
    Value *BigS = B.CreateSelect(Swap, UB.Sign, UA.Sign);
    Value *BigE = B.CreateSelect(Swap, UB.Exp, UA.Exp);
    Value *BigM = B.CreateSelect(Swap, UB.Sig, UA.Sig);
    Value *SmlS = B.CreateSelect(Swap, UA.Sign, UB.Sign);
    Value *SmlE = B.CreateSelect(Swap, UA.Exp, UB.Exp);
    Value *SmlM = B.CreateSelect(Swap, UA.Sig, UB.Sig);
    Value *Aligned = shrSticky(SmlM, B.CreateSub(BigE, SmlE));
    Value *Same = B.CreateICmpEQ(BigS, SmlS);
    Value *Raw = B.CreateSelect(Same, B.CreateAdd(BigM, Aligned),
                                B.CreateSub(BigM, Aligned));
    auto [M, Delta] = normalize(Raw, F.top());
    Value *Res = pack(F, BigS, B.CreateAdd(BigE, Delta), M);
    Res = B.CreateSelect(B.CreateICmpEQ(Raw, c(0)), c(0), Res);
    Res = B.CreateSelect(UB.IsZero, A, Res);
    Res = B.CreateSelect(
        UA.IsZero, B.CreateSelect(UB.IsZero, B.CreateAnd(A, Bv), Bv), Res);
    Res = B.CreateSelect(UB.IsInf, Bv, Res);
    Res = B.CreateSelect(UA.IsInf, A, Res);
    Value *InfClash = B.CreateAnd(B.CreateAnd(UA.IsInf, UB.IsInf),
                                  B.CreateICmpNE(UA.Sign, UB.Sign));
    Value *NaN = B.CreateOr(B.CreateOr(UA.IsNaN, UB.IsNaN), InfClash);
    return B.CreateSelect(NaN, c(F.qnanBits()), Res);
  }

  Value *mul(const dangcsaFloatFmt &F, Value *A, Value *Bv) {
    dangcsaUnpacked UA = unpack(F, A);
    dangcsaUnpacked UB = unpack(F, Bv);
    Value *Sign = B.CreateXor(UA.Sign, UB.Sign);
    Value *P =
        B.CreateMul(B.CreateZExt(UA.Sig, I128), B.CreateZExt(UB.Sig, I128));
    Value *Lead = B.CreateSub(c128(127), ctlz(P));
    Value *Shift = B.CreateSub(Lead, c128(F.top()));
    Shift = B.CreateSelect(B.CreateICmpUGT(Shift, c128(127)), c128(127), Shift);
    Value *Kept = B.CreateTrunc(B.CreateLShr(P, Shift), I64);
    Value *Mask = B.CreateSub(B.CreateShl(c128(1), Shift), c128(1));
    Value *Lost =
        B.CreateZExt(B.CreateICmpNE(B.CreateAnd(P, Mask), c128(0)), I64);
    Value *M = B.CreateOr(Kept, Lost);
    Value *LeadE = B.CreateTrunc(Lead, I64);
    Value *E = B.CreateAdd(B.CreateSub(B.CreateAdd(UA.Exp, UB.Exp), cs(F.Bias)),
                           B.CreateSub(LeadE, c(2 * F.top())));
    Value *Res = pack(F, Sign, E, M);
    Value *AnyZero = B.CreateOr(UA.IsZero, UB.IsZero);
    Value *AnyInf = B.CreateOr(UA.IsInf, UB.IsInf);
    Res = B.CreateSelect(AnyZero, signedZero(F, Sign), Res);
    Res = B.CreateSelect(AnyInf, signedInf(F, Sign), Res);
    Value *NaN = B.CreateOr(B.CreateOr(UA.IsNaN, UB.IsNaN),
                            B.CreateAnd(AnyZero, AnyInf));
    return B.CreateSelect(NaN, c(F.qnanBits()), Res);
  }

  Value *div(const dangcsaFloatFmt &F, Value *A, Value *Bv, Function *Fn) {
    LLVMContext &Ctx = B.getContext();
    dangcsaUnpacked UA = unpack(F, A);
    dangcsaUnpacked UB = unpack(F, Bv);
    Value *Sign = B.CreateXor(UA.Sign, UB.Sign);
    Value *Less = B.CreateICmpULT(UA.Sig, UB.Sig);
    Value *Num = B.CreateSelect(Less, B.CreateShl(UA.Sig, c(1)), UA.Sig);
    Value *E = B.CreateAdd(
        B.CreateSub(B.CreateSub(UA.Exp, B.CreateZExt(Less, I64)), UB.Exp),
        cs(F.Bias));
    BasicBlock *Pre = B.GetInsertBlock();
    BasicBlock *Loop = BasicBlock::Create(Ctx, "div.loop", Fn);
    BasicBlock *Done = BasicBlock::Create(Ctx, "div.done", Fn);
    B.CreateBr(Loop);
    B.SetInsertPoint(Loop);
    PHINode *Idx = B.CreatePHI(I64, 2);
    PHINode *Rem = B.CreatePHI(I64, 2);
    PHINode *Quo = B.CreatePHI(I64, 2);
    Value *Ge = B.CreateICmpUGE(Rem, UB.Sig);
    Value *Quo2 = B.CreateOr(B.CreateShl(Quo, c(1)), B.CreateZExt(Ge, I64));
    Value *Rem2 =
        B.CreateShl(B.CreateSelect(Ge, B.CreateSub(Rem, UB.Sig), Rem), c(1));
    Value *Idx2 = B.CreateAdd(Idx, c(1));
    Idx->addIncoming(c(0), Pre);
    Idx->addIncoming(Idx2, Loop);
    Rem->addIncoming(Num, Pre);
    Rem->addIncoming(Rem2, Loop);
    Quo->addIncoming(c(0), Pre);
    Quo->addIncoming(Quo2, Loop);
    B.CreateCondBr(B.CreateICmpULT(Idx2, c(F.top() + 1)), Loop, Done);
    B.SetInsertPoint(Done);
    Value *M = B.CreateOr(Quo2, B.CreateZExt(B.CreateICmpNE(Rem2, c(0)), I64));
    Value *Res = pack(F, Sign, E, M);
    Value *ToZero = B.CreateOr(UA.IsZero, UB.IsInf);
    Value *ToInf = B.CreateOr(UA.IsInf, UB.IsZero);
    Res = B.CreateSelect(ToZero, signedZero(F, Sign), Res);
    Res = B.CreateSelect(ToInf, signedInf(F, Sign), Res);
    Value *NaN = B.CreateOr(B.CreateOr(UA.IsNaN, UB.IsNaN),
                            B.CreateOr(B.CreateAnd(UA.IsInf, UB.IsInf),
                                       B.CreateAnd(UA.IsZero, UB.IsZero)));
    return B.CreateSelect(NaN, c(F.qnanBits()), Res);
  }

  Value *rem(const dangcsaFloatFmt &F, Value *X, Value *Y, Function *Fn) {
    LLVMContext &Ctx = B.getContext();
    dangcsaUnpacked UX = unpack(F, X);
    dangcsaUnpacked UY = unpack(F, Y);
    Value *AX = B.CreateAnd(X, c(F.absMask()));
    Value *AY = B.CreateAnd(Y, c(F.absMask()));
    Value *NaN = B.CreateOr(B.CreateOr(UX.IsNaN, UY.IsNaN),
                            B.CreateOr(UX.IsInf, UY.IsZero));
    Value *KeepX =
        B.CreateOr(B.CreateOr(UX.IsZero, UY.IsInf), B.CreateICmpULT(AX, AY));
    Value *Equal = B.CreateICmpEQ(AX, AY);
    Value *Special = B.CreateOr(B.CreateOr(NaN, KeepX), Equal);
    Value *MX = B.CreateLShr(UX.Sig, c(3));
    Value *MY = B.CreateLShr(UY.Sig, c(3));
    Value *Gap = B.CreateSub(UX.Exp, UY.Exp);
    Gap = B.CreateSelect(B.CreateICmpSGT(Gap, c(0)), Gap, c(0));
    Gap = B.CreateSelect(Special, c(0), Gap);
    BasicBlock *Pre = B.GetInsertBlock();
    BasicBlock *Loop = BasicBlock::Create(Ctx, "rem.loop", Fn);
    BasicBlock *Done = BasicBlock::Create(Ctx, "rem.done", Fn);
    B.CreateCondBr(B.CreateICmpNE(Gap, c(0)), Loop, Done);
    B.SetInsertPoint(Loop);
    PHINode *R = B.CreatePHI(I64, 2);
    PHINode *K = B.CreatePHI(I64, 2);
    Value *R1 = B.CreateSelect(B.CreateICmpUGE(R, MY), B.CreateSub(R, MY), R);
    Value *R2 = B.CreateShl(R1, c(1));
    Value *K2 = B.CreateSub(K, c(1));
    R->addIncoming(MX, Pre);
    R->addIncoming(R2, Loop);
    K->addIncoming(Gap, Pre);
    K->addIncoming(K2, Loop);
    B.CreateCondBr(B.CreateICmpNE(K2, c(0)), Loop, Done);
    B.SetInsertPoint(Done);
    PHINode *RF = B.CreatePHI(I64, 2);
    RF->addIncoming(MX, Pre);
    RF->addIncoming(R2, Loop);
    Value *RL =
        B.CreateSelect(B.CreateICmpUGE(RF, MY), B.CreateSub(RF, MY), RF);
    auto [M, Delta] = normalize(B.CreateShl(RL, c(3)), F.top());
    Value *Res = pack(F, UX.Sign, B.CreateAdd(UY.Exp, Delta), M);
    Value *Zero = signedZero(F, UX.Sign);
    Res = B.CreateSelect(B.CreateICmpEQ(RL, c(0)), Zero, Res);
    Res = B.CreateSelect(Equal, Zero, Res);
    Res = B.CreateSelect(KeepX, X, Res);
    return B.CreateSelect(NaN, c(F.qnanBits()), Res);
  }

  Value *sqrt(const dangcsaFloatFmt &F, Value *X, Function *Fn) {
    LLVMContext &Ctx = B.getContext();
    dangcsaUnpacked U = unpack(F, X);
    Value *Eu = B.CreateSub(U.Exp, cs(F.Bias));
    Value *Odd = B.CreateICmpNE(B.CreateAnd(Eu, c(1)), c(0));
    Value *Mx = B.CreateSelect(Odd, B.CreateShl(U.Sig, c(1)), U.Sig);
    Value *Eu2 = B.CreateSub(Eu, B.CreateZExt(Odd, I64));
    Value *Rad0 = B.CreateShl(B.CreateZExt(Mx, I128), c128(F.top()));
    Value *Bit0 = B.CreateShl(c128(1), c128(2 * F.top()));
    BasicBlock *Pre = B.GetInsertBlock();
    BasicBlock *Loop = BasicBlock::Create(Ctx, "sqrt.loop", Fn);
    BasicBlock *Done = BasicBlock::Create(Ctx, "sqrt.done", Fn);
    B.CreateBr(Loop);
    B.SetInsertPoint(Loop);
    PHINode *Rad = B.CreatePHI(I128, 2);
    PHINode *Root = B.CreatePHI(I128, 2);
    PHINode *Bit = B.CreatePHI(I128, 2);
    Value *Trial = B.CreateAdd(Root, Bit);
    Value *Ge = B.CreateICmpUGE(Rad, Trial);
    Value *Rad2 = B.CreateSelect(Ge, B.CreateSub(Rad, Trial), Rad);
    Value *Half = B.CreateLShr(Root, c128(1));
    Value *Root2 = B.CreateSelect(Ge, B.CreateAdd(Half, Bit), Half);
    Value *Bit2 = B.CreateLShr(Bit, c128(2));
    Rad->addIncoming(Rad0, Pre);
    Rad->addIncoming(Rad2, Loop);
    Root->addIncoming(c128(0), Pre);
    Root->addIncoming(Root2, Loop);
    Bit->addIncoming(Bit0, Pre);
    Bit->addIncoming(Bit2, Loop);
    B.CreateCondBr(B.CreateICmpNE(Bit2, c128(0)), Loop, Done);
    B.SetInsertPoint(Done);
    Value *M = B.CreateOr(B.CreateTrunc(Root2, I64),
                          B.CreateZExt(B.CreateICmpNE(Rad2, c128(0)), I64));
    Value *E = B.CreateAdd(B.CreateAShr(Eu2, c(1)), cs(F.Bias));
    Value *Res = pack(F, c(0), E, M);
    Res = B.CreateSelect(U.IsInf, X, Res);
    Res = B.CreateSelect(U.IsZero, X, Res);
    Value *Negative =
        B.CreateAnd(B.CreateICmpNE(U.Sign, c(0)), B.CreateNot(U.IsZero));
    Value *NaN = B.CreateOr(U.IsNaN, Negative);
    return B.CreateSelect(NaN, c(F.qnanBits()), Res);
  }

  Value *round(const dangcsaFloatFmt &F, dangcsaRound Mode, Value *X) {
    Value *Exp = B.CreateAnd(B.CreateLShr(X, c(F.FracBits)), c(F.ExpMax));
    Value *Eu = B.CreateSub(Exp, cs(F.Bias));
    Value *Abs = B.CreateAnd(X, c(F.absMask()));
    Value *Sgn = B.CreateAnd(X, c(F.signBit()));
    Value *Neg = B.CreateICmpNE(Sgn, c(0));
    Value *Pos = B.CreateNot(Neg);
    Value *NonZero = B.CreateICmpNE(Abs, c(0));
    Value *IsInt = B.CreateICmpSGE(Eu, c(F.FracBits));
    Value *Small = B.CreateICmpSLT(Eu, c(0));
    Value *HalfOrMore = B.CreateICmpEQ(Eu, cs(-1));
    Value *SmallUp = B.getFalse();
    switch (Mode) {
    case dangcsaRound::Trunc:
      break;
    case dangcsaRound::Floor:
      SmallUp = B.CreateAnd(Neg, NonZero);
      break;
    case dangcsaRound::Ceil:
      SmallUp = B.CreateAnd(Pos, NonZero);
      break;
    case dangcsaRound::Away:
      SmallUp = HalfOrMore;
      break;
    case dangcsaRound::Even:
      SmallUp = B.CreateAnd(
          HalfOrMore,
          B.CreateICmpUGT(Abs, c(uint64_t(F.Bias - 1) << F.FracBits)));
      break;
    }
    Value *SmallRes =
        B.CreateOr(Sgn, B.CreateSelect(SmallUp, c(F.oneBits()), c(0)));
    Value *Amt = B.CreateSelect(B.CreateICmpSLT(Eu, c(0)), c(0), Eu);
    Amt = B.CreateSelect(B.CreateICmpUGT(Amt, c(63)), c(63), Amt);
    Value *Mask = B.CreateLShr(c(F.fracMask()), Amt);
    Value *Frac = B.CreateAnd(X, Mask);
    Value *Base = B.CreateAnd(X, B.CreateNot(Mask));
    Value *Unit = B.CreateAdd(Mask, c(1));
    Value *Half = B.CreateLShr(Unit, c(1));
    Value *FracNonZero = B.CreateICmpNE(Frac, c(0));
    Value *MidUp = B.getFalse();
    switch (Mode) {
    case dangcsaRound::Trunc:
      break;
    case dangcsaRound::Floor:
      MidUp = B.CreateAnd(Neg, FracNonZero);
      break;
    case dangcsaRound::Ceil:
      MidUp = B.CreateAnd(Pos, FracNonZero);
      break;
    case dangcsaRound::Away:
      MidUp = B.CreateICmpUGE(Frac, Half);
      break;
    case dangcsaRound::Even:
      MidUp = B.CreateOr(
          B.CreateICmpUGT(Frac, Half),
          B.CreateAnd(B.CreateICmpEQ(Frac, Half),
                      B.CreateICmpNE(B.CreateAnd(Base, Unit), c(0))));
      break;
    }
    Value *MidRes = B.CreateSelect(MidUp, B.CreateAdd(Base, Unit), Base);
    return B.CreateSelect(IsInt, X, B.CreateSelect(Small, SmallRes, MidRes));
  }

  Value *fromInt(const dangcsaFloatFmt &F, Value *V, bool Signed) {
    Value *Neg = Signed ? B.CreateICmpSLT(V, c(0)) : B.getFalse();
    Value *Mag = B.CreateSelect(Neg, B.CreateNeg(V), V);
    auto [M, Delta] = normalize(Mag, F.top());
    Value *E = B.CreateAdd(Delta, c(uint64_t(F.Bias) + F.top()));
    Value *Res = pack(F, B.CreateZExt(Neg, I64), E, M);
    return B.CreateSelect(B.CreateICmpEQ(Mag, c(0)), c(0), Res);
  }

  Value *toInt(const dangcsaFloatFmt &F, Value *X, bool Signed) {
    Value *Exp = B.CreateAnd(B.CreateLShr(X, c(F.FracBits)), c(F.ExpMax));
    Value *Sig =
        B.CreateOr(B.CreateAnd(X, c(F.fracMask())), c(F.implicitBit()));
    Value *Sh = B.CreateSub(Exp, c(uint64_t(F.Bias) + F.FracBits));
    Value *Left = B.CreateICmpSGE(Sh, c(0));
    Value *LAmt = B.CreateSelect(Left, Sh, c(0));
    LAmt = B.CreateSelect(B.CreateICmpUGT(LAmt, c(63)), c(63), LAmt);
    Value *RAmt = B.CreateSelect(Left, c(0), B.CreateNeg(Sh));
    RAmt = B.CreateSelect(B.CreateICmpUGT(RAmt, c(63)), c(63), RAmt);
    Value *V =
        B.CreateSelect(Left, B.CreateShl(Sig, LAmt), B.CreateLShr(Sig, RAmt));
    V = B.CreateSelect(B.CreateICmpULT(Exp, c(F.Bias)), c(0), V);
    if (!Signed)
      return V;
    Value *Neg = B.CreateICmpNE(B.CreateAnd(X, c(F.signBit())), c(0));
    return B.CreateSelect(Neg, B.CreateNeg(V), V);
  }

  Value *convert(const dangcsaFloatFmt &S, const dangcsaFloatFmt &D, Value *X) {
    dangcsaUnpacked U = unpack(S, X);
    auto [M, Delta] = normalize(U.Sig, D.top());
    int64_t Rebase = D.Bias + int64_t(D.top()) - S.Bias - int64_t(S.top());
    Value *E = B.CreateAdd(B.CreateAdd(U.Exp, Delta), cs(Rebase));
    Value *Res = pack(D, U.Sign, E, M);
    Res = B.CreateSelect(U.IsZero, signedZero(D, U.Sign), Res);
    Res = B.CreateSelect(U.IsInf, signedInf(D, U.Sign), Res);
    return B.CreateSelect(U.IsNaN, c(D.qnanBits()), Res);
  }

  Value *cmp(const dangcsaFloatFmt &F, CmpInst::Predicate P, Value *A,
             Value *Bv) {
    Value *AAbs = B.CreateAnd(A, c(F.absMask()));
    Value *BAbs = B.CreateAnd(Bv, c(F.absMask()));
    Value *Uno = B.CreateOr(B.CreateICmpUGT(AAbs, c(F.infBits())),
                            B.CreateICmpUGT(BAbs, c(F.infBits())));
    Value *Ord = B.CreateNot(Uno);
    Value *BothZero = B.CreateICmpEQ(B.CreateOr(AAbs, BAbs), c(0));
    Value *NotZero = B.CreateNot(BothZero);
    Value *AKey = key(F, A, AAbs);
    Value *BKey = key(F, Bv, BAbs);
    Value *Eq = B.CreateOr(BothZero, B.CreateICmpEQ(A, Bv));
    Value *Lt = B.CreateAnd(NotZero, B.CreateICmpSLT(AKey, BKey));
    Value *Gt = B.CreateAnd(NotZero, B.CreateICmpSGT(AKey, BKey));
    switch (P) {
    case CmpInst::FCMP_FALSE:
      return B.getFalse();
    case CmpInst::FCMP_TRUE:
      return B.getTrue();
    case CmpInst::FCMP_OEQ:
      return B.CreateAnd(Ord, Eq);
    case CmpInst::FCMP_OGT:
      return B.CreateAnd(Ord, Gt);
    case CmpInst::FCMP_OGE:
      return B.CreateAnd(Ord, B.CreateOr(Gt, Eq));
    case CmpInst::FCMP_OLT:
      return B.CreateAnd(Ord, Lt);
    case CmpInst::FCMP_OLE:
      return B.CreateAnd(Ord, B.CreateOr(Lt, Eq));
    case CmpInst::FCMP_ONE:
      return B.CreateAnd(Ord, B.CreateNot(Eq));
    case CmpInst::FCMP_ORD:
      return Ord;
    case CmpInst::FCMP_UNO:
      return Uno;
    case CmpInst::FCMP_UEQ:
      return B.CreateOr(Uno, Eq);
    case CmpInst::FCMP_UGT:
      return B.CreateOr(Uno, Gt);
    case CmpInst::FCMP_UGE:
      return B.CreateOr(Uno, B.CreateOr(Gt, Eq));
    case CmpInst::FCMP_ULT:
      return B.CreateOr(Uno, Lt);
    case CmpInst::FCMP_ULE:
      return B.CreateOr(Uno, B.CreateOr(Lt, Eq));
    case CmpInst::FCMP_UNE:
      return B.CreateOr(Uno, B.CreateNot(Eq));
    default:
      report_fatal_error("dangcsa: unknown floating-point predicate");
    }
  }

  Value *minMax(const dangcsaFloatFmt &F, bool IsMin, bool QuietNaN, Value *A,
                Value *Bv) {
    Value *AAbs = B.CreateAnd(A, c(F.absMask()));
    Value *BAbs = B.CreateAnd(Bv, c(F.absMask()));
    Value *ANaN = B.CreateICmpUGT(AAbs, c(F.infBits()));
    Value *BNaN = B.CreateICmpUGT(BAbs, c(F.infBits()));
    Value *ALt = B.CreateICmpSLT(orderKey(F, A, AAbs), orderKey(F, Bv, BAbs));
    Value *Pick =
        IsMin ? B.CreateSelect(ALt, A, Bv) : B.CreateSelect(ALt, Bv, A);
    if (!QuietNaN)
      return B.CreateSelect(B.CreateOr(ANaN, BNaN), c(F.qnanBits()), Pick);
    Value *IfANaN = B.CreateSelect(BNaN, c(F.qnanBits()), Bv);
    Value *IfANum = B.CreateSelect(BNaN, A, Pick);
    return B.CreateSelect(ANaN, IfANaN, IfANum);
  }

  Value *fpClass(const dangcsaFloatFmt &F, Value *X, uint64_t Test) {
    Value *Abs = B.CreateAnd(X, c(F.absMask()));
    Value *Neg = B.CreateICmpNE(B.CreateAnd(X, c(F.signBit())), c(0));
    Value *Pos = B.CreateNot(Neg);
    Value *Exp = B.CreateAnd(B.CreateLShr(X, c(F.FracBits)), c(F.ExpMax));
    Value *IsNaN = B.CreateICmpUGT(Abs, c(F.infBits()));
    Value *Quiet = B.CreateICmpNE(B.CreateAnd(X, c(F.quietBit())), c(0));
    Value *IsInf = B.CreateICmpEQ(Abs, c(F.infBits()));
    Value *IsZero = B.CreateICmpEQ(Abs, c(0));
    Value *IsSub = B.CreateAnd(B.CreateICmpEQ(Exp, c(0)), B.CreateNot(IsZero));
    Value *IsNorm = B.CreateAnd(B.CreateICmpNE(Exp, c(0)),
                                B.CreateICmpNE(Exp, c(F.ExpMax)));
    Value *Classes[10] = {B.CreateAnd(IsNaN, B.CreateNot(Quiet)),
                          B.CreateAnd(IsNaN, Quiet),
                          B.CreateAnd(Neg, IsInf),
                          B.CreateAnd(Neg, IsNorm),
                          B.CreateAnd(Neg, IsSub),
                          B.CreateAnd(Neg, IsZero),
                          B.CreateAnd(Pos, IsZero),
                          B.CreateAnd(Pos, IsSub),
                          B.CreateAnd(Pos, IsNorm),
                          B.CreateAnd(Pos, IsInf)};
    Value *Res = B.getFalse();
    for (unsigned Idx = 0; Idx < 10; ++Idx)
      if (Test & (uint64_t(1) << Idx))
        Res = B.CreateOr(Res, Classes[Idx]);
    return Res;
  }

private:
  Value *key(const dangcsaFloatFmt &F, Value *X, Value *Abs) {
    Value *Neg = B.CreateICmpNE(B.CreateAnd(X, c(F.signBit())), c(0));
    return B.CreateSelect(Neg, B.CreateNeg(Abs), Abs);
  }

  Value *orderKey(const dangcsaFloatFmt &F, Value *X, Value *Abs) {
    Value *Neg = B.CreateICmpNE(B.CreateAnd(X, c(F.signBit())), c(0));
    return B.CreateSelect(Neg, B.CreateSub(B.CreateNeg(Abs), c(1)), Abs);
  }

  IRBuilder<> &B;
  Type *I64;
  Type *I128;
};

class dangcsaFloatExpand : public ModulePass {
public:
  static char ID;

  dangcsaFloatExpand() : ModulePass(ID) {}

  StringRef getPassName() const override {
    return "dangcsa floating-point expansion";
  }

  bool runOnModule(Module &M) override;

private:
  using BodyFn = function_ref<Value *(IRBuilder<> &, Function *)>;

  Function *getFn(const Twine &Name, FunctionType *FT, BodyFn Body);
  Function *arith(const dangcsaFloatFmt &F, StringRef Op);
  Function *unary(const dangcsaFloatFmt &F, StringRef Op);
  Function *conv(const dangcsaFloatFmt &S, const dangcsaFloatFmt &D);
  Function *toInt(const dangcsaFloatFmt &F, bool Signed);
  Function *fromInt(const dangcsaFloatFmt &F, bool Signed);
  Function *powi(const dangcsaFloatFmt &F);

  static bool wants(Instruction &I);
  const dangcsaFloatFmt &fmtOf(Type *Ty, const Instruction &I);
  Value *expand(IRBuilder<> &B, Instruction &I);
  Value *expandIntrinsic(IRBuilder<> &B, IntrinsicInst &II);
  Value *callBinary(IRBuilder<> &B, Function *Fn, Value *A, Value *Bv,
                    Type *Ty);
  Value *callUnary(IRBuilder<> &B, Function *Fn, Value *A, Type *Ty);

  static Value *toBits(IRBuilder<> &B, Value *V);
  static Value *fromBits(IRBuilder<> &B, Value *V, Type *Ty);

  Module *Mod = nullptr;
};

} // namespace

char dangcsaFloatExpand::ID = 0;

INITIALIZE_PASS(dangcsaFloatExpand, DEBUG_TYPE,
                "dangcsa floating-point expansion", false, false)

ModulePass *llvm::createdangcsaFloatExpandPass() {
  return new dangcsaFloatExpand();
}

Value *dangcsaFloatExpand::toBits(IRBuilder<> &B, Value *V) {
  Type *Ty = V->getType();
  Value *Raw = B.CreateBitCast(V, B.getIntNTy(Ty->getScalarSizeInBits()));
  return B.CreateZExtOrBitCast(Raw, B.getInt64Ty());
}

Value *dangcsaFloatExpand::fromBits(IRBuilder<> &B, Value *V, Type *Ty) {
  Value *Raw =
      B.CreateTruncOrBitCast(V, B.getIntNTy(Ty->getScalarSizeInBits()));
  return B.CreateBitCast(Raw, Ty);
}

Function *dangcsaFloatExpand::getFn(const Twine &Name, FunctionType *FT,
                                    BodyFn Body) {
  std::string Key = Name.str();
  if (Function *Existing = Mod->getFunction(Key))
    return Existing;
  Function *Fn = Function::Create(FT, GlobalValue::InternalLinkage, Key, Mod);
  Fn->addFnAttr(Attribute::NoUnwind);
  Fn->addFnAttr(Attribute::WillReturn);
  Fn->setDoesNotAccessMemory();
  BasicBlock *Entry = BasicBlock::Create(Mod->getContext(), "entry", Fn);
  IRBuilder<> B(Entry);
  Value *Ret = Body(B, Fn);
  B.CreateRet(Ret);
  return Fn;
}

Function *dangcsaFloatExpand::arith(const dangcsaFloatFmt &F, StringRef Op) {
  Type *I64 = Type::getInt64Ty(Mod->getContext());
  FunctionType *FT = FunctionType::get(I64, {I64, I64}, false);
  return getFn("dangcsa.fp." + Op + "." + F.Tag, FT,
               [&](IRBuilder<> &B, Function *Fn) -> Value * {
                 dangcsaFloatGen G(B);
                 Value *A = Fn->getArg(0);
                 Value *Bv = Fn->getArg(1);
                 if (Op == "add")
                   return G.add(F, A, Bv);
                 if (Op == "sub")
                   return G.add(F, A, B.CreateXor(Bv, G.c(F.signBit())));
                 if (Op == "mul")
                   return G.mul(F, A, Bv);
                 if (Op == "div")
                   return G.div(F, A, Bv, Fn);
                 return G.rem(F, A, Bv, Fn);
               });
}

Function *dangcsaFloatExpand::unary(const dangcsaFloatFmt &F, StringRef Op) {
  Type *I64 = Type::getInt64Ty(Mod->getContext());
  FunctionType *FT = FunctionType::get(I64, {I64}, false);
  return getFn("dangcsa.fp." + Op + "." + F.Tag, FT,
               [&](IRBuilder<> &B, Function *Fn) -> Value * {
                 dangcsaFloatGen G(B);
                 Value *X = Fn->getArg(0);
                 if (Op == "sqrt")
                   return G.sqrt(F, X, Fn);
                 if (Op == "floor")
                   return G.round(F, dangcsaRound::Floor, X);
                 if (Op == "ceil")
                   return G.round(F, dangcsaRound::Ceil, X);
                 if (Op == "trunc")
                   return G.round(F, dangcsaRound::Trunc, X);
                 if (Op == "round")
                   return G.round(F, dangcsaRound::Away, X);
                 return G.round(F, dangcsaRound::Even, X);
               });
}

Function *dangcsaFloatExpand::conv(const dangcsaFloatFmt &S,
                                   const dangcsaFloatFmt &D) {
  Type *I64 = Type::getInt64Ty(Mod->getContext());
  FunctionType *FT = FunctionType::get(I64, {I64}, false);
  return getFn(Twine("dangcsa.fp.conv.") + S.Tag + "." + D.Tag, FT,
               [&](IRBuilder<> &B, Function *Fn) -> Value * {
                 dangcsaFloatGen G(B);
                 return G.convert(S, D, Fn->getArg(0));
               });
}

Function *dangcsaFloatExpand::toInt(const dangcsaFloatFmt &F, bool Signed) {
  Type *I64 = Type::getInt64Ty(Mod->getContext());
  FunctionType *FT = FunctionType::get(I64, {I64}, false);
  return getFn(Twine("dangcsa.fp.toint.") + (Signed ? "s." : "u.") + F.Tag, FT,
               [&](IRBuilder<> &B, Function *Fn) -> Value * {
                 dangcsaFloatGen G(B);
                 return G.toInt(F, Fn->getArg(0), Signed);
               });
}

Function *dangcsaFloatExpand::fromInt(const dangcsaFloatFmt &F, bool Signed) {
  Type *I64 = Type::getInt64Ty(Mod->getContext());
  FunctionType *FT = FunctionType::get(I64, {I64}, false);
  return getFn(Twine("dangcsa.fp.fromint.") + (Signed ? "s." : "u.") + F.Tag,
               FT, [&](IRBuilder<> &B, Function *Fn) -> Value * {
                 dangcsaFloatGen G(B);
                 return G.fromInt(F, Fn->getArg(0), Signed);
               });
}

Function *dangcsaFloatExpand::powi(const dangcsaFloatFmt &F) {
  LLVMContext &Ctx = Mod->getContext();
  Type *I64 = Type::getInt64Ty(Ctx);
  Type *I32 = Type::getInt32Ty(Ctx);
  FunctionType *FT = FunctionType::get(I64, {I64, I32}, false);
  Function *Mul = arith(F, "mul");
  Function *Div = arith(F, "div");
  return getFn(Twine("dangcsa.fp.powi.") + F.Tag, FT,
               [&](IRBuilder<> &B, Function *Fn) -> Value * {
                 dangcsaFloatGen G(B);
                 Value *X = Fn->getArg(0);
                 Value *N = B.CreateSExt(Fn->getArg(1), I64);
                 Value *Neg = B.CreateICmpSLT(N, G.c(0));
                 Value *Count = B.CreateSelect(Neg, B.CreateNeg(N), N);
                 BasicBlock *Pre = B.GetInsertBlock();
                 BasicBlock *Loop = BasicBlock::Create(Ctx, "powi.loop", Fn);
                 BasicBlock *Done = BasicBlock::Create(Ctx, "powi.done", Fn);
                 B.CreateCondBr(B.CreateICmpNE(Count, G.c(0)), Loop, Done);
                 B.SetInsertPoint(Loop);
                 PHINode *Acc = B.CreatePHI(I64, 2);
                 PHINode *Base = B.CreatePHI(I64, 2);
                 PHINode *Left = B.CreatePHI(I64, 2);
                 Value *Odd = B.CreateICmpNE(B.CreateAnd(Left, G.c(1)), G.c(0));
                 Value *Acc2 =
                     B.CreateSelect(Odd, B.CreateCall(Mul, {Acc, Base}), Acc);
                 Value *Base2 = B.CreateCall(Mul, {Base, Base});
                 Value *Left2 = B.CreateLShr(Left, G.c(1));
                 Acc->addIncoming(G.c(F.oneBits()), Pre);
                 Acc->addIncoming(Acc2, Loop);
                 Base->addIncoming(X, Pre);
                 Base->addIncoming(Base2, Loop);
                 Left->addIncoming(Count, Pre);
                 Left->addIncoming(Left2, Loop);
                 B.CreateCondBr(B.CreateICmpNE(Left2, G.c(0)), Loop, Done);
                 B.SetInsertPoint(Done);
                 PHINode *Res = B.CreatePHI(I64, 2);
                 Res->addIncoming(G.c(F.oneBits()), Pre);
                 Res->addIncoming(Acc2, Loop);
                 Value *Inv = B.CreateCall(Div, {G.c(F.oneBits()), Res});
                 return B.CreateSelect(Neg, Inv, Res);
               });
}

bool dangcsaFloatExpand::wants(Instruction &I) {
  switch (I.getOpcode()) {
  case Instruction::FAdd:
  case Instruction::FSub:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
  case Instruction::FCmp:
  case Instruction::FPToSI:
  case Instruction::FPToUI:
  case Instruction::SIToFP:
  case Instruction::UIToFP:
  case Instruction::FPExt:
  case Instruction::FPTrunc:
    return true;
  case Instruction::Call:
    break;
  default:
    return false;
  }
  auto *II = dyn_cast<IntrinsicInst>(&I);
  if (!II)
    return false;
  switch (II->getIntrinsicID()) {
  case Intrinsic::fabs:
  case Intrinsic::copysign:
  case Intrinsic::fake_use:
    return false;
  default:
    break;
  }
  if (I.getType()->isFPOrFPVectorTy())
    return true;
  for (const Use &Op : I.operands())
    if (Op->getType()->isFPOrFPVectorTy())
      return true;
  return false;
}

const dangcsaFloatFmt &dangcsaFloatExpand::fmtOf(Type *Ty,
                                                 const Instruction &I) {
  if (Ty->isFloatTy())
    return dangcsaFmtF32;
  if (Ty->isDoubleTy())
    return dangcsaFmtF64;
  report_fatal_error(Twine("dangcsa: floating-point type not supported in ") +
                     I.getFunction()->getName());
}

Value *dangcsaFloatExpand::callBinary(IRBuilder<> &B, Function *Fn, Value *A,
                                      Value *Bv, Type *Ty) {
  Value *R = B.CreateCall(Fn, {toBits(B, A), toBits(B, Bv)});
  return fromBits(B, R, Ty);
}

Value *dangcsaFloatExpand::callUnary(IRBuilder<> &B, Function *Fn, Value *A,
                                     Type *Ty) {
  return fromBits(B, B.CreateCall(Fn, {toBits(B, A)}), Ty);
}

Value *dangcsaFloatExpand::expand(IRBuilder<> &B, Instruction &I) {
  switch (I.getOpcode()) {
  case Instruction::FAdd:
    return callBinary(B, arith(fmtOf(I.getType(), I), "add"), I.getOperand(0),
                      I.getOperand(1), I.getType());
  case Instruction::FSub:
    return callBinary(B, arith(fmtOf(I.getType(), I), "sub"), I.getOperand(0),
                      I.getOperand(1), I.getType());
  case Instruction::FMul:
    return callBinary(B, arith(fmtOf(I.getType(), I), "mul"), I.getOperand(0),
                      I.getOperand(1), I.getType());
  case Instruction::FDiv:
    return callBinary(B, arith(fmtOf(I.getType(), I), "div"), I.getOperand(0),
                      I.getOperand(1), I.getType());
  case Instruction::FRem:
    return callBinary(B, arith(fmtOf(I.getType(), I), "rem"), I.getOperand(0),
                      I.getOperand(1), I.getType());
  case Instruction::FCmp: {
    auto &C = cast<FCmpInst>(I);
    const dangcsaFloatFmt &F = fmtOf(C.getOperand(0)->getType(), I);
    dangcsaFloatGen G(B);
    return G.cmp(F, C.getPredicate(), toBits(B, C.getOperand(0)),
                 toBits(B, C.getOperand(1)));
  }
  case Instruction::FPToSI:
  case Instruction::FPToUI: {
    bool Signed = I.getOpcode() == Instruction::FPToSI;
    const dangcsaFloatFmt &F = fmtOf(I.getOperand(0)->getType(), I);
    if (!I.getType()->isIntegerTy() || I.getType()->getIntegerBitWidth() > 64)
      report_fatal_error(
          "dangcsa: unsupported floating-point to integer conversion");
    Value *R = B.CreateCall(toInt(F, Signed), {toBits(B, I.getOperand(0))});
    return B.CreateTruncOrBitCast(R, I.getType());
  }
  case Instruction::SIToFP:
  case Instruction::UIToFP: {
    bool Signed = I.getOpcode() == Instruction::SIToFP;
    const dangcsaFloatFmt &F = fmtOf(I.getType(), I);
    Type *SrcTy = I.getOperand(0)->getType();
    if (!SrcTy->isIntegerTy() || SrcTy->getIntegerBitWidth() > 64)
      report_fatal_error(
          "dangcsa: unsupported integer to floating-point conversion");
    Value *Wide = Signed
                      ? B.CreateSExtOrBitCast(I.getOperand(0), B.getInt64Ty())
                      : B.CreateZExtOrBitCast(I.getOperand(0), B.getInt64Ty());
    return fromBits(B, B.CreateCall(fromInt(F, Signed), {Wide}), I.getType());
  }
  case Instruction::FPExt:
  case Instruction::FPTrunc: {
    const dangcsaFloatFmt &S = fmtOf(I.getOperand(0)->getType(), I);
    const dangcsaFloatFmt &D = fmtOf(I.getType(), I);
    return callUnary(B, conv(S, D), I.getOperand(0), I.getType());
  }
  case Instruction::Call:
    return expandIntrinsic(B, cast<IntrinsicInst>(I));
  default:
    llvm_unreachable("dangcsa: unexpected floating-point instruction");
  }
}

Value *dangcsaFloatExpand::expandIntrinsic(IRBuilder<> &B, IntrinsicInst &II) {
  Type *Ty = II.getType();
  switch (II.getIntrinsicID()) {
  case Intrinsic::sqrt:
    return callUnary(B, unary(fmtOf(Ty, II), "sqrt"), II.getArgOperand(0), Ty);
  case Intrinsic::floor:
    return callUnary(B, unary(fmtOf(Ty, II), "floor"), II.getArgOperand(0), Ty);
  case Intrinsic::ceil:
    return callUnary(B, unary(fmtOf(Ty, II), "ceil"), II.getArgOperand(0), Ty);
  case Intrinsic::trunc:
    return callUnary(B, unary(fmtOf(Ty, II), "trunc"), II.getArgOperand(0), Ty);
  case Intrinsic::round:
    return callUnary(B, unary(fmtOf(Ty, II), "round"), II.getArgOperand(0), Ty);
  case Intrinsic::roundeven:
  case Intrinsic::rint:
  case Intrinsic::nearbyint:
    return callUnary(B, unary(fmtOf(Ty, II), "rint"), II.getArgOperand(0), Ty);
  case Intrinsic::minnum:
  case Intrinsic::maxnum:
  case Intrinsic::minimum:
  case Intrinsic::maximum:
  case Intrinsic::minimumnum:
  case Intrinsic::maximumnum: {
    Intrinsic::ID ID = II.getIntrinsicID();
    bool IsMin = ID == Intrinsic::minnum || ID == Intrinsic::minimum ||
                 ID == Intrinsic::minimumnum;
    bool Quiet = ID != Intrinsic::minimum && ID != Intrinsic::maximum;
    dangcsaFloatGen G(B);
    Value *R =
        G.minMax(fmtOf(Ty, II), IsMin, Quiet, toBits(B, II.getArgOperand(0)),
                 toBits(B, II.getArgOperand(1)));
    return fromBits(B, R, Ty);
  }
  case Intrinsic::powi: {
    Value *N = B.CreateSExtOrTrunc(II.getArgOperand(1), B.getInt32Ty());
    Value *R =
        B.CreateCall(powi(fmtOf(Ty, II)), {toBits(B, II.getArgOperand(0)), N});
    return fromBits(B, R, Ty);
  }
  case Intrinsic::fmuladd: {
    const dangcsaFloatFmt &F = fmtOf(Ty, II);
    Value *P = callBinary(B, arith(F, "mul"), II.getArgOperand(0),
                          II.getArgOperand(1), Ty);
    return callBinary(B, arith(F, "add"), P, II.getArgOperand(2), Ty);
  }
  case Intrinsic::is_fpclass: {
    Value *X = II.getArgOperand(0);
    uint64_t Test = cast<ConstantInt>(II.getArgOperand(1))->getZExtValue();
    dangcsaFloatGen G(B);
    return G.fpClass(fmtOf(X->getType(), II), toBits(B, X), Test);
  }
  case Intrinsic::canonicalize:
  case Intrinsic::arithmetic_fence:
    return II.getArgOperand(0);
  default:
    report_fatal_error(
        Twine("dangcsa: floating-point intrinsic not supported: ") +
        II.getCalledFunction()->getName());
  }
}

bool dangcsaFloatExpand::runOnModule(Module &M) {
  Mod = &M;
  SmallVector<Instruction *, 64> Work;
  for (Function &F : M) {
    if (F.isDeclaration())
      continue;
    for (Instruction &I : instructions(F))
      if (wants(I))
        Work.push_back(&I);
  }
  for (Instruction *I : Work) {
    IRBuilder<> B(I);
    Value *New = expand(B, *I);
    I->replaceAllUsesWith(New);
    I->eraseFromParent();
  }
  return !Work.empty();
}
