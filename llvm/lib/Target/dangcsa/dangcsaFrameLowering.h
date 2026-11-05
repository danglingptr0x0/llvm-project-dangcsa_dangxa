#ifndef LLVM_LIB_TARGET_DANGCSA_DANGCSAFRAMELOWERING_H
#define LLVM_LIB_TARGET_DANGCSA_DANGCSAFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/Alignment.h"

namespace llvm {

class dangcsaSubtarget;

class dangcsaFrameLowering : public TargetFrameLowering {
public:
  explicit dangcsaFrameLowering(const dangcsaSubtarget &STI)
      : TargetFrameLowering(StackGrowsDown, Align(8), 0, Align(8)),
        Subtarget(STI) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFPImpl(const MachineFunction &MF) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

private:
  const dangcsaSubtarget &Subtarget;
};

} // namespace llvm

#endif
