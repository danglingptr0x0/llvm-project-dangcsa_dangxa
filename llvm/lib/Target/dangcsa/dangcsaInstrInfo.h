#ifndef LLVM_LIB_TARGET_DANGCSA_DANGCSAINSTRINFO_H
#define LLVM_LIB_TARGET_DANGCSA_DANGCSAINSTRINFO_H

#include "dangcsaRegisterInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define GET_INSTRINFO_HEADER
#include "dangcsaGenInstrInfo.inc"

namespace llvm {

class dangcsaInstrInfo : public dangcsaGenInstrInfo {
public:
  dangcsaInstrInfo(const TargetSubtargetInfo &STI,
                   const TargetRegisterInfo &TRI);

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                   const DebugLoc &DL, Register DstReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
      bool IsKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register DstReg,
      int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      unsigned SubIdx = 0,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;
};

} // namespace llvm

#endif
