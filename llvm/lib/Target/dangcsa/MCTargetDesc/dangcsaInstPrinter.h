#ifndef LLVM_LIB_TARGET_DANGCSA_MCTARGETDESC_DANGCSAINSTPRINTER_H
#define LLVM_LIB_TARGET_DANGCSA_MCTARGETDESC_DANGCSAINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCRegister.h"
#include <cstdint>
#include <utility>

namespace llvm {

class MCAsmInfo;
class MCInst;
class MCInstrInfo;
class MCRegisterInfo;
class MCSubtargetInfo;
class raw_ostream;

class dangcsaInstPrinter : public MCInstPrinter {
public:
  dangcsaInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                     const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override;

  void printRegName(raw_ostream &O, MCRegister Reg) override;

  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override;

  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);

  bool printAliasInstr(const MCInst *MI, uint64_t Address, raw_ostream &O);

  static const char *getRegisterName(MCRegister Reg);

  void printBranchTarget(const MCInst *MI, uint64_t Addr, uint32_t OpNo,
                         raw_ostream &OS);

  void printSImm32(const MCInst *MI, uint32_t OpNo, raw_ostream &OS);

  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printUImm16(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printUImm6(const MCInst *MI, unsigned OpNo, raw_ostream &O);
};

} // namespace llvm

#endif
