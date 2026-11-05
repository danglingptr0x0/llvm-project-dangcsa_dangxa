#ifndef LLVM_LIB_TARGET_DANGCSA_DANGCSASUBTARGET_H
#define LLVM_LIB_TARGET_DANGCSA_DANGCSASUBTARGET_H

#include "dangcsaFrameLowering.h"
#include "dangcsaISelLowering.h"
#include "dangcsaInstrInfo.h"
#include "dangcsaRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/TargetParser/Triple.h"

#define GET_SUBTARGETINFO_HEADER
#include "dangcsaGenSubtargetInfo.inc"

namespace llvm {

class StringRef;
class TargetMachine;

class dangcsaSubtarget : public dangcsaGenSubtargetInfo {
  virtual void anchor();

  dangcsaRegisterInfo RegInfo;
  dangcsaInstrInfo InstrInfo;
  dangcsaFrameLowering FrameLowering;
  dangcsaTargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;

public:
  dangcsaSubtarget(const Triple &TT, StringRef CPU, StringRef TuneCPU,
                   StringRef FS, const TargetMachine &TM);

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  const dangcsaInstrInfo *getInstrInfo() const override { return &InstrInfo; }

  const dangcsaFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }

  const dangcsaRegisterInfo *getRegisterInfo() const override {
    return &RegInfo;
  }

  const dangcsaTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
};

} // namespace llvm

#endif
