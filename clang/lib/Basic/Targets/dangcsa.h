#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_DANGCSA_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_DANGCSA_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY dangcsaTargetInfo : public TargetInfo {
public:
  dangcsaTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    BigEndian = false;
    NoAsmVariants = true;
    TLSSupported = false;
    HasFloat16 = false;

    IntWidth = IntAlign = 32;
    LongWidth = LongAlign = 64;
    LongLongWidth = LongLongAlign = 64;
    PointerWidth = PointerAlign = 64;
    SuitableAlign = 64;

    FloatWidth = 32;
    FloatAlign = 32;
    DoubleWidth = LongDoubleWidth = 64;
    DoubleAlign = LongDoubleAlign = 64;
    LongDoubleFormat = &llvm::APFloat::IEEEdouble();

    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    IntMaxType = SignedLong;
    WCharType = SignedInt;
    WIntType = SignedInt;
    Char16Type = UnsignedShort;
    Char32Type = UnsignedInt;
    SigAtomicType = SignedInt;

    resetDataLayout();
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  void adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
              const TargetInfo *Aux) override;

  llvm::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  bool hasFeature(StringRef Feature) const override {
    return Feature == "dangcsa" || Feature == "dangcsa64";
  }

  ArrayRef<const char *> getGCCRegNames() const override {
    static const char *const GCCRegNames[] = {
        "dr0",  "dr1",  "dr2",  "dr3",  "dr4",  "dr5",  "dr6",  "dr7",
        "dr8",  "dr9",  "dr10", "dr11", "dr12", "dr13", "dr14", "dr15",
        "tdr0", "tdr1", "tdr2", "tdr3", "tdr4", "tdr5", "loc",  "sp"};
    return llvm::ArrayRef(GCCRegNames);
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    case 'r':
      Info.setAllowsRegister();
      return true;
    default:
      return false;
    }
  }

  bool hasBitIntType() const override { return true; }

  bool isCLZForZeroUndef() const override { return false; }
};

} // namespace targets
} // namespace clang

#endif
