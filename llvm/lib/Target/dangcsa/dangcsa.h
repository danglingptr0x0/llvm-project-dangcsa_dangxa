#ifndef LLVM_LIB_TARGET_DANGCSA_DANGCSA_H
#define LLVM_LIB_TARGET_DANGCSA_DANGCSA_H

#include "MCTargetDesc/dangcsaMCTargetDesc.h"
#include "llvm/Pass.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class AsmPrinter;
class FunctionPass;
class ModulePass;
class MachineInstr;
class MachineOperand;
class MCInst;
class MCOperand;
class dangcsaTargetMachine;
class PassRegistry;

namespace dangcsaII {

enum TOF { MO_NONE = 0, MO_PCREL, MO_Q0, MO_Q1, MO_Q2, MO_Q3 };

}

FunctionPass *createdangcsaISelDag(dangcsaTargetMachine &TM,
                                   CodeGenOptLevel OptLevel);

void initializedangcsaDAGToDAGISelLegacyPass(PassRegistry &);

ModulePass *createdangcsaFloatExpandPass();

void initializedangcsaFloatExpandPass(PassRegistry &);

void lowerdangcsaMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                      AsmPrinter &AP);

MCOperand lowerdangcsaSymbolOperand(const MachineOperand &MO, AsmPrinter &AP);

} // namespace llvm

#endif
