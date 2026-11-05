#include "dangcsaMCAsmInfo.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

dangcsa::Specifier dangcsa::parseSpecifierName(StringRef Name) {
  return StringSwitch<dangcsa::Specifier>(Name)
      .Case("pcrel", dangcsa::S_PCREL)
      .Case("q0", dangcsa::S_Q0)
      .Case("q1", dangcsa::S_Q1)
      .Case("q2", dangcsa::S_Q2)
      .Case("q3", dangcsa::S_Q3)
      .Default(dangcsa::S_NONE);
}

StringRef dangcsa::getSpecifierName(dangcsa::Specifier Kind) {
  switch (Kind) {
  case dangcsa::S_PCREL:
    return "pcrel";
  case dangcsa::S_Q0:
    return "q0";
  case dangcsa::S_Q1:
    return "q1";
  case dangcsa::S_Q2:
    return "q2";
  case dangcsa::S_Q3:
    return "q3";
  case dangcsa::S_NONE:
    break;
  }
  llvm_unreachable("invalid DANGCSA/DANGXA relocation specifier");
}

void dangcsaMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                          const MCSpecifierExpr &Expr) const {
  dangcsa::Specifier Kind = dangcsa::Specifier(Expr.getSpecifier());

  if (Kind != dangcsa::S_NONE)
    OS << '%' << dangcsa::getSpecifierName(Kind) << '(';

  printExpr(OS, *Expr.getSubExpr());

  if (Kind != dangcsa::S_NONE)
    OS << ')';
}

void dangcsaMCAsmInfo::anchor() {}

dangcsaMCAsmInfo::dangcsaMCAsmInfo(const Triple &TT,
                                   const MCTargetOptions &Options)
    : MCAsmInfoELF(Options) {
  CodePointerSize = 8;
  CalleeSaveStackSlotSize = 8;
  IsLittleEndian = true;
  CommentString = ";";
  AlignmentIsInBytes = false;
  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::None;
  Data8bitsDirective = "\t.byte\t";
  Data16bitsDirective = "\t.short\t";
  Data32bitsDirective = "\t.long\t";
  Data64bitsDirective = "\t.quad\t";
  ZeroDirective = "\t.zero\t";
  AsciiDirective = "\t.ascii\t";
  AscizDirective = "\t.asciz\t";
  UsesELFSectionDirectiveForBSS = true;
}
