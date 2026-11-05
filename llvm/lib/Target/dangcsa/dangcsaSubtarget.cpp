#include "dangcsaSubtarget.h"
#include "dangcsa.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "dangcsa-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "dangcsaGenSubtargetInfo.inc"

void dangcsaSubtarget::anchor() {}

dangcsaSubtarget::dangcsaSubtarget(const Triple &TT, StringRef CPU,
                                   StringRef TuneCPU, StringRef FS,
                                   const TargetMachine &TM)
    : dangcsaGenSubtargetInfo(TT, CPU, TuneCPU, FS), RegInfo(),
      InstrInfo(*this, RegInfo), FrameLowering(*this), TLInfo(TM, *this) {
  ParseSubtargetFeatures(CPU, TuneCPU, FS);
}
