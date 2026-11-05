#include "MCTargetDesc/dangcsaFixupKinds.h"
#include "MCTargetDesc/dangcsaMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {

class dangcsaELFObjectWriter : public MCELFObjectTargetWriter {
public:
  dangcsaELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(true, OSABI, ELF::EM_DANGCSA, true) {}

  ~dangcsaELFObjectWriter() override = default;

protected:
  unsigned getRelocType(const MCFixup &Fixup, const MCValue &Target,
                        bool IsPCRel) const override;
};

} // namespace

unsigned dangcsaELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                              const MCValue &Target,
                                              bool IsPCRel) const {
  switch (Fixup.getKind()) {
  case dangcsa::fixup_dangcsa_imm32:
    return ELF::R_DANGCSA_IMM32;
  case dangcsa::fixup_dangcsa_pcrel32:
    return ELF::R_DANGCSA_PCREL32;
  case dangcsa::fixup_dangcsa_branch32:
    return ELF::R_DANGCSA_BRANCH32;
  case dangcsa::fixup_dangcsa_q0:
    return ELF::R_DANGCSA_Q0;
  case dangcsa::fixup_dangcsa_q1:
    return ELF::R_DANGCSA_Q1;
  case dangcsa::fixup_dangcsa_q2:
    return ELF::R_DANGCSA_Q2;
  case dangcsa::fixup_dangcsa_q3:
    return ELF::R_DANGCSA_Q3;
  case FK_Data_8:
    return IsPCRel ? ELF::R_DANGCSA_REL64 : ELF::R_DANGCSA_64;
  case FK_Data_4:
    return IsPCRel ? ELF::R_DANGCSA_REL32 : ELF::R_DANGCSA_32;
  case FK_Data_1:
  case FK_Data_2:
    reportError(Fixup.getLoc(),
                "unsupported relocation width for DANGCSA/DANGXA");
    return ELF::R_DANGCSA_NONE;
  default:
    llvm_unreachable("unsupported relocation type");
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createdangcsaELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<dangcsaELFObjectWriter>(OSABI);
}
