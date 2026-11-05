#ifndef LLVM_LIB_TARGET_DANGCSA_DANGCSAREGISTERINFO_H
#define LLVM_LIB_TARGET_DANGCSA_DANGCSAREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "dangcsaGenRegisterInfo.inc"

namespace llvm {

struct dangcsaRegisterInfo : public dangcsaGenRegisterInfo {
  dangcsaRegisterInfo();

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CallConv) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override;

  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override;
};

} // namespace llvm

#endif
