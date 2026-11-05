#include "MCTargetDesc/dangcsaInstPrinter.h"
#include "TargetInfo/dangcsaTargetInfo.h"
#include "dangcsa.h"
#include "dangcsaSubtarget.h"
#include "dangcsaTargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "dangcsa-asm-printer"

namespace {

class dangcsaAsmPrinter : public AsmPrinter {
public:
  explicit dangcsaAsmPrinter(TargetMachine &TM,
                             std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID) {}

  static char ID;

  StringRef getPassName() const override { return "dangcsa Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;
  void emitPCAddr(const MachineInstr *MI);
};

} // namespace

char dangcsaAsmPrinter::ID = 0;

void dangcsaAsmPrinter::emitPCAddr(const MachineInstr *MI) {
  MCOperand Dst = MCOperand::createReg(MI->getOperand(0).getReg());
  MCOperand PcrelSym;
  MachineOperand Sym = MI->getOperand(1);
  MCInst Inst;

  Sym.setTargetFlags(dangcsaII::MO_PCREL);
  PcrelSym = lowerdangcsaSymbolOperand(Sym, *this);

  Inst.setOpcode(dangcsa::CPY);
  Inst.addOperand(Dst);
  Inst.addOperand(MCOperand::createReg(dangcsa::LOC));
  EmitToStreamer(*OutStreamer, Inst);

  Inst.clear();
  Inst.setOpcode(dangcsa::ADD_ri);
  Inst.addOperand(Dst);
  Inst.addOperand(Dst);
  Inst.addOperand(PcrelSym);
  EmitToStreamer(*OutStreamer, Inst);
}

void dangcsaAsmPrinter::emitInstruction(const MachineInstr *MI) {
  if (MI->getOpcode() == dangcsa::PCADDR) {
    emitPCAddr(MI);
    return;
  }

  MCInst TmpInst;
  lowerdangcsaMachineInstrToMCInst(MI, TmpInst, *this);
  EmitToStreamer(*OutStreamer, TmpInst);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializedangcsaAsmPrinter() {
  RegisterAsmPrinter<dangcsaAsmPrinter> X(getThedangcsa64Target());
}
