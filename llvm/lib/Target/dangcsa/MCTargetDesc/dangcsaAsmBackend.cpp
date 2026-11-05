#include "MCTargetDesc/dangcsaFixupKinds.h"
#include "MCTargetDesc/dangcsaMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {

class dangcsaAsmBackend : public MCAsmBackend {
  uint8_t OSABI;

public:
  dangcsaAsmBackend(uint8_t OSABI)
      : MCAsmBackend(llvm::endianness::little), OSABI(OSABI) {}

  unsigned getNumFixupKinds() const { return dangcsa::NumTargetFixupKinds; }

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override {
    const static MCFixupKindInfo Infos[dangcsa::NumTargetFixupKinds] = {
        {"fixup_dangcsa_imm32", 0, 32, 0},
        {"fixup_dangcsa_pcrel32", 0, 32, 0},
        {"fixup_dangcsa_branch32", 0, 32, 0},
        {"fixup_dangcsa_q0", 0, 16, 0},
        {"fixup_dangcsa_q1", 0, 16, 0},
        {"fixup_dangcsa_q2", 0, 16, 0},
        {"fixup_dangcsa_q3", 0, 16, 0}};

    if (Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);

    return Infos[Kind - FirstTargetFixupKind];
  }

  void applyFixup(const MCFragment &F, const MCFixup &Fixup,
                  const MCValue &Target, uint8_t *Data, uint64_t Value,
                  bool IsResolved) override;

  bool writeNopData(raw_ostream &OS, uint64_t Cunt,
                    const MCSubtargetInfo *STI) const override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createdangcsaELFObjectWriter(OSABI);
  }
};

} // namespace

void dangcsaAsmBackend::applyFixup(const MCFragment &F, const MCFixup &Fixup,
                                   const MCValue &Target, uint8_t *Data,
                                   uint64_t Value, bool IsResolved) {
  unsigned Kind = Fixup.getKind();
  bool PCRel = Fixup.isPCRel() || Kind == dangcsa::fixup_dangcsa_pcrel32 ||
               Kind == dangcsa::fixup_dangcsa_branch32;

  if (!IsResolved && !PCRel && Target.getAddSym() && !Target.getSubSym()) {
    const MCSectionELF *Sec = static_cast<const MCSectionELF *>(F.getParent());

    if ((Sec->getFlags() & ELF::SHF_ALLOC) && Sec->getName() != ".eh_frame") {
      getContext().reportError(Fixup.getLoc(),
                               Twine("absolute address of '") +
                                   Target.getAddSym()->getName() +
                                   "' in section '" + Sec->getName() +
                                   "' requires relocation (OC Std 15.4)");
      return;
    }
  }

  maybeAddReloc(F, Fixup, Target, Value, IsResolved);

  if (!Value)
    return;

  uint64_t Insert = 0;
  unsigned NumBytes = 0;

  if (Kind == FK_Data_1 || Kind == FK_Data_2 || Kind == FK_Data_4 ||
      Kind == FK_Data_8) {
    NumBytes = 1u << (Kind - FK_Data_1);

    for (unsigned i = 0; i != NumBytes; ++i)
      Data[i] |= static_cast<uint8_t>((Value >> (i * 8)) & 0xFF);

    return;
  }

  switch (Kind) {
  case dangcsa::fixup_dangcsa_imm32:
    Insert = Value & 0xFFFFFFFF;
    NumBytes = 4;
    break;
  case dangcsa::fixup_dangcsa_pcrel32:
    Insert = (Value + 8) & 0xFFFFFFFF;
    NumBytes = 4;
    break;
  case dangcsa::fixup_dangcsa_branch32:
    Insert = (static_cast<int64_t>(Value) >> 3) & 0xFFFFFFFF;
    NumBytes = 4;
    break;
  case dangcsa::fixup_dangcsa_q0:
    Insert = Value & 0xFFFF;
    NumBytes = 2;
    break;
  case dangcsa::fixup_dangcsa_q1:
    Insert = (Value >> 16) & 0xFFFF;
    NumBytes = 2;
    break;
  case dangcsa::fixup_dangcsa_q2:
    Insert = (Value >> 32) & 0xFFFF;
    NumBytes = 2;
    break;
  case dangcsa::fixup_dangcsa_q3:
    Insert = (Value >> 48) & 0xFFFF;
    NumBytes = 2;
    break;
  default:
    llvm_unreachable("unknown fixup kind");
  }

  for (unsigned i = 0; i != NumBytes; ++i)
    Data[i] |= static_cast<uint8_t>((Insert >> (i * 8)) & 0xFF);
}

bool dangcsaAsmBackend::writeNopData(raw_ostream &OS, uint64_t Cunt,
                                     const MCSubtargetInfo *STI) const {
  if ((Cunt % 8) != 0)
    return false;

  for (uint64_t i = 0; i < Cunt; i += 8)
    support::endian::write<uint64_t>(OS, 0, llvm::endianness::little);

  return true;
}

MCAsmBackend *llvm::createdangcsaAsmBackend(const Target &T,
                                            const MCSubtargetInfo &STI,
                                            const MCRegisterInfo &MRI,
                                            const MCTargetOptions &Options) {
  return new dangcsaAsmBackend(ELF::ELFOSABI_DANGLING);
}
