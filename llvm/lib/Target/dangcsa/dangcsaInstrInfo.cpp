#include "dangcsaInstrInfo.h"
#include "dangcsa.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "dangcsaGenInstrInfo.inc"

using namespace llvm;

static const unsigned BranchTgtIdx = 2;
static const unsigned CondWidth = 3;
static const unsigned InstrBytes = 8;

static bool isCondBranchOpcode(unsigned Opc) {
  switch (Opc) {
  case dangcsa::BEQ:
  case dangcsa::BNE:
  case dangcsa::BLT:
  case dangcsa::BGE:
  case dangcsa::BLTU:
  case dangcsa::BGEU:
    return true;
  default:
    return false;
  }
}

static bool isUncondBranch(const MachineInstr &MI) {
  return MI.getOpcode() == dangcsa::JREL;
}

dangcsaInstrInfo::dangcsaInstrInfo(const TargetSubtargetInfo &STI,
                                   const TargetRegisterInfo &TRI)
    : dangcsaGenInstrInfo(STI, TRI, dangcsa::ADJCALLSTACKDOWN,
                          dangcsa::ADJCALLSTACKUP, ~0u, dangcsa::RET) {}

void dangcsaInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MBBI,
                                   const DebugLoc &DL, Register DstReg,
                                   Register SrcReg, bool KillSrc,
                                   bool RenamableDest,
                                   bool RenamableSrc) const {
  if (!dangcsa::GPRRegClass.contains(DstReg, SrcReg))
    report_fatal_error("dangcsa copyPhysReg supports GPR only");

  BuildMI(MBB, MBBI, DL, get(dangcsa::CPY), DstReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
}

void dangcsaInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
    bool IsKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (MBBI != MBB.end())
    DL = MBBI->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOStore, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlign(FrameIndex));

  BuildMI(MBB, MBBI, DL, get(dangcsa::STQ))
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addReg(SrcReg, getKillRegState(IsKill))
      .addMemOperand(MMO);
}

void dangcsaInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                            MachineBasicBlock::iterator MBBI,
                                            Register DstReg, int FrameIndex,
                                            const TargetRegisterClass *RC,
                                            Register VReg, unsigned SubIdx,
                                            MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (MBBI != MBB.end())
    DL = MBBI->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOLoad, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlign(FrameIndex));

  BuildMI(MBB, MBBI, DL, get(dangcsa::LDQ), DstReg)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(MMO);
}

bool dangcsaInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                     MachineBasicBlock *&TBB,
                                     MachineBasicBlock *&FBB,
                                     SmallVectorImpl<MachineOperand> &Cond,
                                     bool AllowModify) const {
  TBB = nullptr;
  FBB = nullptr;
  Cond.clear();

  MachineBasicBlock::iterator Iter = MBB.getLastNonDebugInstr();
  if (Iter == MBB.end())
    return false;

  if (!isUnpredicatedTerminator(*Iter))
    return false;

  MachineInstr *LastInst = &*Iter;

  if (Iter == MBB.begin() || !isUnpredicatedTerminator(*(--Iter))) {
    if (isUncondBranch(*LastInst)) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    }

    if (isCondBranchOpcode(LastInst->getOpcode())) {
      TBB = LastInst->getOperand(BranchTgtIdx).getMBB();
      Cond.push_back(MachineOperand::CreateImm(LastInst->getOpcode()));
      Cond.push_back(LastInst->getOperand(0));
      Cond.push_back(LastInst->getOperand(1));
      return false;
    }

    return true;
  }

  MachineInstr *PriorInst = &*Iter;

  if (Iter != MBB.begin() && isUnpredicatedTerminator(*(--Iter)))
    return true;

  if (!isUncondBranch(*LastInst))
    return true;

  if (isUncondBranch(*PriorInst) || !isCondBranchOpcode(PriorInst->getOpcode()))
    return true;

  TBB = PriorInst->getOperand(BranchTgtIdx).getMBB();
  Cond.push_back(MachineOperand::CreateImm(PriorInst->getOpcode()));
  Cond.push_back(PriorInst->getOperand(0));
  Cond.push_back(PriorInst->getOperand(1));
  FBB = LastInst->getOperand(0).getMBB();

  return false;
}

unsigned dangcsaInstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  if (BytesAdded)
    *BytesAdded = 0;

  if (Cond.empty()) {
    BuildMI(&MBB, DL, get(dangcsa::JREL)).addMBB(TBB);

    if (BytesAdded)
      *BytesAdded += InstrBytes;

    return 1;
  }

  BuildMI(&MBB, DL, get(Cond[0].getImm()))
      .add(Cond[1])
      .add(Cond[2])
      .addMBB(TBB);

  if (BytesAdded)
    *BytesAdded += InstrBytes;

  if (!FBB)
    return 1;

  BuildMI(&MBB, DL, get(dangcsa::JREL)).addMBB(FBB);

  if (BytesAdded)
    *BytesAdded += InstrBytes;

  return 2;
}

unsigned dangcsaInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                        int *BytesRemoved) const {
  if (BytesRemoved)
    *BytesRemoved = 0;

  MachineBasicBlock::instr_iterator Iter = MBB.instr_end();
  unsigned Cunt = 0;

  while (Iter != MBB.instr_begin()) {
    --Iter;

    if (Iter->isDebugInstr())
      continue;

    if (!isCondBranchOpcode(Iter->getOpcode()) && !isUncondBranch(*Iter))
      break;

    Iter->eraseFromParent();
    Iter = MBB.instr_end();
    Cunt++;

    if (BytesRemoved)
      *BytesRemoved += InstrBytes;
  }

  return Cunt;
}

bool dangcsaInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  if (Cond.size() != CondWidth)
    return true;

  unsigned Opc = Cond[0].getImm();

  switch (Opc) {
  case dangcsa::BEQ:
    Opc = dangcsa::BNE;
    break;
  case dangcsa::BNE:
    Opc = dangcsa::BEQ;
    break;
  case dangcsa::BLT:
    Opc = dangcsa::BGE;
    break;
  case dangcsa::BGE:
    Opc = dangcsa::BLT;
    break;
  case dangcsa::BLTU:
    Opc = dangcsa::BGEU;
    break;
  case dangcsa::BGEU:
    Opc = dangcsa::BLTU;
    break;
  default:
    return true;
  }

  Cond[0].setImm(Opc);

  return false;
}
