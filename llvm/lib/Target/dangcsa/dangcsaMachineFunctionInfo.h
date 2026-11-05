#ifndef LLVM_LIB_TARGET_DANGCSA_DANGCSAMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_DANGCSA_DANGCSAMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class dangcsaMachineFunctionInfo : public MachineFunctionInfo {
  int VarArgsFrameIndex = 0;

public:
  dangcsaMachineFunctionInfo(const Function &F,
                             const TargetSubtargetInfo *STI) {}

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }

  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
};

} // namespace llvm

#endif
