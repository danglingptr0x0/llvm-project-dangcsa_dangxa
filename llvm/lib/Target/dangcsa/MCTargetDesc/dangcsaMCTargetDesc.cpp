#include "dangcsaMCTargetDesc.h"
#include "TargetInfo/dangcsaTargetInfo.h"
#include "dangcsaInstPrinter.h"
#include "dangcsaMCAsmInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Triple.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "dangcsaGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "dangcsaGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "dangcsaGenSubtargetInfo.inc"

using namespace llvm;

static MCInstrInfo *createdangcsaMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitdangcsaMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createdangcsaMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitdangcsaMCRegisterInfo(X, dangcsa::SP);
  return X;
}

static MCAsmInfo *createdangcsaMCAsmInfo(const MCRegisterInfo &MRI,
                                         const Triple &TT,
                                         const MCTargetOptions &Options) {
  return new dangcsaMCAsmInfo(TT, Options);
}

static MCSubtargetInfo *
createdangcsaMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  if (CPU.empty())
    CPU = "generic-dangcsa";

  return createdangcsaMCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

static MCInstPrinter *createdangcsaMCInstPrinter(const Triple &T,
                                                 unsigned SyntaxVariant,
                                                 const MCAsmInfo &MAI,
                                                 const MCInstrInfo &MII,
                                                 const MCRegisterInfo &MRI) {
  return new dangcsaInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializedangcsaTargetMC() {
  Target &T = getThedangcsa64Target();

  TargetRegistry::RegisterMCAsmInfo(T, createdangcsaMCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(T, createdangcsaMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(T, createdangcsaMCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createdangcsaMCSubtargetInfo);
  TargetRegistry::RegisterMCInstPrinter(T, createdangcsaMCInstPrinter);
  TargetRegistry::RegisterMCCodeEmitter(T, createdangcsaMCCodeEmitter);
  TargetRegistry::RegisterMCAsmBackend(T, createdangcsaAsmBackend);
}
