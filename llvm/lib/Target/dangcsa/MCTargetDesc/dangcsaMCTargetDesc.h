#ifndef LLVM_LIB_TARGET_DANGCSA_MCTARGETDESC_DANGCSAMCTARGETDESC_H
#define LLVM_LIB_TARGET_DANGCSA_MCTARGETDESC_DANGCSAMCTARGETDESC_H

#include "llvm/MC/MCContext.h"
#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {

class MCAsmBackend;
class MCCodeEmitter;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCCodeEmitter *createdangcsaMCCodeEmitter(const MCInstrInfo &MCII,
                                          MCContext &Ctx);

MCAsmBackend *createdangcsaAsmBackend(const Target &T,
                                      const MCSubtargetInfo &STI,
                                      const MCRegisterInfo &MRI,
                                      const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter>
createdangcsaELFObjectWriter(uint8_t OSABI);

} // namespace llvm

#define GET_REGINFO_ENUM
#include "dangcsaGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "dangcsaGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "dangcsaGenSubtargetInfo.inc"

#endif
