#include "dangcsaRegisterInfo.h"
#include "dangcsa.h"
#include "dangcsaFrameLowering.h"
#include "dangcsaInstrInfo.h"
#include "dangcsaSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "dangcsaGenRegisterInfo.inc"

using namespace llvm;

dangcsaRegisterInfo::dangcsaRegisterInfo()
    : dangcsaGenRegisterInfo(dangcsa::DR0) {}

const MCPhysReg *
dangcsaRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_dangcsa_SaveList;
}

const uint32_t *
dangcsaRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                          CallingConv::ID CallConv) const {
  return CSR_dangcsa_RegMask;
}

BitVector
dangcsaRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(dangcsa::TDR0);
  Reserved.set(dangcsa::LOC);
  Reserved.set(dangcsa::SP);

  return Reserved;
}

Register
dangcsaRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return dangcsa::SP;
}

bool dangcsaRegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return true;
}

bool dangcsaRegisterInfo::requiresFrameIndexScavenging(
    const MachineFunction &MF) const {
  return true;
}

static void materializeFrameAddr(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator II,
                                 const DebugLoc &DL,
                                 const dangcsaInstrInfo &TII, Register Rd,
                                 int64_t Offset) {
  BuildMI(MBB, II, DL, TII.get(dangcsa::CPY_ri), Rd).addImm(Offset);

  BuildMI(MBB, II, DL, TII.get(dangcsa::ADD), Rd)
      .addReg(Rd)
      .addReg(dangcsa::SP);
}

bool dangcsaRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                              int SPAdj, unsigned FIOperandNum,
                                              RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  int64_t Offset = MFI.getObjectOffset(FrameIndex) + MFI.getStackSize();

  const MachineOperand &DispMO = MI.getOperand(FIOperandNum + 1);
  Offset += DispMO.getImm();

  MachineBasicBlock &MBB = *MI.getParent();
  const dangcsaInstrInfo &TII =
      *MF.getSubtarget<dangcsaSubtarget>().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  if (MI.getOpcode() == dangcsa::FRAMEADDR) {
    Register Rd = MI.getOperand(0).getReg();

    materializeFrameAddr(MBB, II, DL, TII, Rd, Offset);

    MI.eraseFromParent();
    return true;
  }

  if (!isInt<32>(Offset)) {
    Register ScratchReg = dangcsa::TDR0;

    materializeFrameAddr(MBB, II, DL, TII, ScratchReg, Offset);

    MI.getOperand(FIOperandNum)
        .ChangeToRegister(ScratchReg, false, false, true);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(0);

    return false;
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(dangcsa::SP, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);

  return false;
}
