#include "MCTargetDesc/dangcsaMCTargetDesc.h"
#include "TargetInfo/dangcsaTargetInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDecoder.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::MCD;

#define DEBUG_TYPE "dangcsa-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

class dangcsaDisassembler : public MCDisassembler {
public:
  dangcsaDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

} // namespace

static MCDisassembler *createdangcsaDisassembler(const Target &T,
                                                 const MCSubtargetInfo &STI,
                                                 MCContext &Ctx) {
  return new dangcsaDisassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializedangcsaDisassembler() {
  TargetRegistry::RegisterMCDisassembler(getThedangcsa64Target(),
                                         createdangcsaDisassembler);
}

static const MCPhysReg GPRDecoderTable[] = {
    dangcsa::DR0,  dangcsa::DR1,  dangcsa::DR2,  dangcsa::DR3,  dangcsa::DR4,
    dangcsa::DR5,  dangcsa::DR6,  dangcsa::DR7,  dangcsa::DR8,  dangcsa::DR9,
    dangcsa::DR10, dangcsa::DR11, dangcsa::DR12, dangcsa::DR13, dangcsa::DR14,
    dangcsa::DR15, dangcsa::TDR0, dangcsa::TDR1, dangcsa::TDR2, dangcsa::TDR3,
    dangcsa::TDR4, dangcsa::TDR5, dangcsa::LOC,  dangcsa::SP};

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  if (RegNo >= 24)
    return MCDisassembler::Fail;

  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[RegNo]));
  return MCDisassembler::Success;
}

static DecodeStatus decodeUImm16(MCInst &Inst, uint64_t Imm, int64_t Address,
                                 const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createImm(Imm & 0xFFFF));
  return MCDisassembler::Success;
}

static DecodeStatus decodeUImm6(MCInst &Inst, uint64_t Imm, int64_t Address,
                                const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createImm(Imm & 0x3F));
  return MCDisassembler::Success;
}

static DecodeStatus decodeSImm32(MCInst &Inst, uint64_t Imm, int64_t Address,
                                 const MCDisassembler *Decoder) {
  int64_t Value = static_cast<int64_t>(static_cast<int32_t>(Imm & 0xFFFFFFFF));
  Inst.addOperand(MCOperand::createImm(Value));
  return MCDisassembler::Success;
}

static DecodeStatus decodeBranchTarget(MCInst &Inst, uint64_t Imm,
                                       int64_t Address,
                                       const MCDisassembler *Decoder) {
  int64_t Offset = static_cast<int64_t>(static_cast<int32_t>(Imm & 0xFFFFFFFF));
  Inst.addOperand(MCOperand::createImm(Offset * 8));
  return MCDisassembler::Success;
}

#include "dangcsaGenDisassemblerTables.inc"

DecodeStatus dangcsaDisassembler::getInstruction(MCInst &MI, uint64_t &Size,
                                                 ArrayRef<uint8_t> Bytes,
                                                 uint64_t Address,
                                                 raw_ostream &CS) const {
  if (Bytes.size() < 8) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = 8;
  uint64_t Insn = support::endian::read64le(Bytes.data());

  return decodeInstruction(DecoderTable64, MI, Insn, Address, this, STI);
}
