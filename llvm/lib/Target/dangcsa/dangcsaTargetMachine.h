#ifndef LLVM_LIB_TARGET_DANGCSA_DANGCSATARGETMACHINE_H
#define LLVM_LIB_TARGET_DANGCSA_DANGCSATARGETMACHINE_H

#include "dangcsaSubtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Target/TargetMachine.h"
#include <optional>

namespace llvm {

class dangcsaTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  dangcsaSubtarget Subtarget;

public:
  dangcsaTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);

  const dangcsaSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &Fn,
                            const TargetSubtargetInfo *STI) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // namespace llvm

#endif
