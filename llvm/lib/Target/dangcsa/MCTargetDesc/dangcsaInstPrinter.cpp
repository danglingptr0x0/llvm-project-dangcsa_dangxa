#include "dangcsaInstPrinter.h"
#include "MCTargetDesc/dangcsaMCTargetDesc.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "dangcsa-asm-printer"

#define PRINT_ALIAS_INSTR
#include "dangcsaGenAsmWriter.inc"

void dangcsaInstPrinter::printRegName(raw_ostream &O, MCRegister Reg) {
  O << getRegisterName(Reg);
}

void dangcsaInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                   StringRef Annot, const MCSubtargetInfo &STI,
                                   raw_ostream &O) {
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void dangcsaInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                      raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    O << MO.getImm();
    return;
  }

  assert(MO.isExpr() && "unexpected operand kind");
  MAI.printExpr(O, *MO.getExpr());
}

void dangcsaInstPrinter::printSImm32(const MCInst *MI, uint32_t OpNo,
                                     raw_ostream &OS) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isImm()) {
    OS << MO.getImm();
    return;
  }

  MAI.printExpr(OS, *MO.getExpr());
}

void dangcsaInstPrinter::printBranchTarget(const MCInst *MI, uint64_t Addr,
                                           uint32_t OpNo, raw_ostream &OS) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isImm()) {
    OS << MO.getImm();
    return;
  }

  MAI.printExpr(OS, *MO.getExpr());
}

void dangcsaInstPrinter::printUImm16(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isImm()) {
    O << formatHex(static_cast<uint64_t>(MO.getImm() & 0xFFFF));
    return;
  }

  printOperand(MI, OpNo, O);
}

void dangcsaInstPrinter::printUImm6(const MCInst *MI, unsigned OpNo,
                                    raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isImm()) {
    O << (MO.getImm() & 0x3F);
    return;
  }

  printOperand(MI, OpNo, O);
}
