#include "MCTargetDesc/dangcsaFixupKinds.h"
#include "MCTargetDesc/dangcsaMCAsmInfo.h"
#include "MCTargetDesc/dangcsaMCTargetDesc.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class dangcsaMCCodeEmitter : public MCCodeEmitter {
  MCContext &Ctx;
  const MCInstrInfo &MCII;

public:
  dangcsaMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx)
      : Ctx(Ctx), MCII(MCII) {}

  dangcsaMCCodeEmitter(const dangcsaMCCodeEmitter &) = delete;
  dangcsaMCCodeEmitter &operator=(const dangcsaMCCodeEmitter &) = delete;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  uint64_t getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  uint64_t getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups,
                                  const MCSubtargetInfo &STI) const;
};

} // namespace

void dangcsaMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                             SmallVectorImpl<char> &CB,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  uint64_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  support::endian::write(CB, Bits, llvm::endianness::little);
}

uint64_t
dangcsaMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<uint64_t>(MO.getImm());

  assert(MO.isExpr() && "unexpected operand kind");

  const MCExpr *Expr = MO.getExpr();
  MCFixupKind Kind = static_cast<MCFixupKind>(dangcsa::fixup_dangcsa_imm32);
  bool PCRel = false;

  if (const auto *Spec = dyn_cast<MCSpecifierExpr>(Expr)) {
    switch (dangcsa::Specifier(Spec->getSpecifier())) {
    case dangcsa::S_PCREL:
      Kind = static_cast<MCFixupKind>(dangcsa::fixup_dangcsa_pcrel32);
      PCRel = true;
      break;
    case dangcsa::S_Q0:
      Kind = static_cast<MCFixupKind>(dangcsa::fixup_dangcsa_q0);
      break;
    case dangcsa::S_Q1:
      Kind = static_cast<MCFixupKind>(dangcsa::fixup_dangcsa_q1);
      break;
    case dangcsa::S_Q2:
      Kind = static_cast<MCFixupKind>(dangcsa::fixup_dangcsa_q2);
      break;
    case dangcsa::S_Q3:
      Kind = static_cast<MCFixupKind>(dangcsa::fixup_dangcsa_q3);
      break;
    case dangcsa::S_NONE:
      break;
    }
  }

  Fixups.push_back(MCFixup::create(0, Expr, Kind, PCRel));
  return 0;
}

uint64_t
dangcsaMCCodeEmitter::getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  if (MO.isImm())
    return static_cast<uint64_t>(MO.getImm() >> 3) & 0xFFFFFFFF;

  assert(MO.isExpr() && "branch target must be an immediate or expression");

  MCFixupKind Kind = static_cast<MCFixupKind>(dangcsa::fixup_dangcsa_branch32);
  Fixups.push_back(MCFixup::create(0, MO.getExpr(), Kind, true));
  return 0;
}

MCCodeEmitter *llvm::createdangcsaMCCodeEmitter(const MCInstrInfo &MCII,
                                                MCContext &Ctx) {
  return new dangcsaMCCodeEmitter(MCII, Ctx);
}

#include "dangcsaGenMCCodeEmitter.inc"
