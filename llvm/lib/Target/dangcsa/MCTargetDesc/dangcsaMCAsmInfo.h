#ifndef LLVM_LIB_TARGET_DANGCSA_MCTARGETDESC_DANGCSAMCASMINFO_H
#define LLVM_LIB_TARGET_DANGCSA_MCTARGETDESC_DANGCSAMCASMINFO_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

class Triple;
class MCTargetOptions;
class MCSpecifierExpr;
class raw_ostream;

namespace dangcsa {

enum Specifier { S_NONE = 0, S_PCREL, S_Q0, S_Q1, S_Q2, S_Q3 };

Specifier parseSpecifierName(StringRef Name);
StringRef getSpecifierName(Specifier Kind);

} // namespace dangcsa

class dangcsaMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  dangcsaMCAsmInfo(const Triple &TT, const MCTargetOptions &Options);

  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
};

} // namespace llvm

#endif
