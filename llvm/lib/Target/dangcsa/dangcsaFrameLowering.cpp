#include "dangcsaFrameLowering.h"
#include "dangcsa.h"
#include "dangcsaInstrInfo.h"
#include "dangcsaSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

static void adjustStackPtr(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI, const DebugLoc &DL,
                           const dangcsaInstrInfo &TII, int64_t Delta) {
  if (Delta == 0)
    return;

  if (isInt<32>(Delta)) {
    BuildMI(MBB, MBBI, DL, TII.get(dangcsa::ADD_ri), dangcsa::SP)
        .addReg(dangcsa::SP)
        .addImm(Delta);
    return;
  }

  BuildMI(MBB, MBBI, DL, TII.get(dangcsa::CPYQ3_ri), dangcsa::TDR0)
      .addReg(dangcsa::TDR0, RegState::Undef)
      .addImm((Delta >> 48) & 0xFFFF);
  BuildMI(MBB, MBBI, DL, TII.get(dangcsa::CPYQ2_ri), dangcsa::TDR0)
      .addReg(dangcsa::TDR0)
      .addImm((Delta >> 32) & 0xFFFF);
  BuildMI(MBB, MBBI, DL, TII.get(dangcsa::CPYQ1_ri), dangcsa::TDR0)
      .addReg(dangcsa::TDR0)
      .addImm((Delta >> 16) & 0xFFFF);
  BuildMI(MBB, MBBI, DL, TII.get(dangcsa::CPYQ0_ri), dangcsa::TDR0)
      .addReg(dangcsa::TDR0)
      .addImm(Delta & 0xFFFF);
  BuildMI(MBB, MBBI, DL, TII.get(dangcsa::ADD), dangcsa::SP)
      .addReg(dangcsa::SP)
      .addReg(dangcsa::TDR0, RegState::Kill);
}

bool dangcsaFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  return false;
}

void dangcsaFrameLowering::emitPrologue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const dangcsaInstrInfo &TII =
      *static_cast<const dangcsaInstrInfo *>(MF.getSubtarget().getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL;

  uint64_t StackSize = MFI.getStackSize();
  if (StackSize == 0)
    return;

  adjustStackPtr(MBB, MBBI, DL, TII, -static_cast<int64_t>(StackSize));
}

void dangcsaFrameLowering::emitEpilogue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const dangcsaInstrInfo &TII =
      *static_cast<const dangcsaInstrInfo *>(MF.getSubtarget().getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.getFirstTerminator();
  DebugLoc DL;
  if (MBBI != MBB.end())
    DL = MBBI->getDebugLoc();

  uint64_t StackSize = MFI.getStackSize();
  if (StackSize == 0)
    return;

  adjustStackPtr(MBB, MBBI, DL, TII, static_cast<int64_t>(StackSize));
}

MachineBasicBlock::iterator dangcsaFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  return MBB.erase(MI);
}
