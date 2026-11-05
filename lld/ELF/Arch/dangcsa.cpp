#include "OutputSections.h"
#include "SymbolTable.h"
#include "Symbols.h"
#include "Target.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {

class dangcsa final : public TargetInfo {
public:
  dangcsa(Ctx &);
  RelExpr getRelExpr(RelType type, const Symbol &sym,
                     const uint8_t *loc) const override;
  RelType getDynRel(RelType type) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
  void relocateAlloc(InputSection &sec, uint8_t *buf) const override;
};

} // namespace

dangcsa::dangcsa(Ctx &ctx) : TargetInfo(ctx) {
  trapInstr = {0x00, 0x00, 0xc0, 0x0f};
  relativeRel = R_DANGCSA_RELATIVE;
  symbolicRel = R_DANGCSA_64;
  defaultImageBase = 0x0;
  defaultMaxPageSize = 8;
  defaultCommonPageSize = 8;
}

RelExpr dangcsa::getRelExpr(RelType type, const Symbol &sym,
                            const uint8_t *loc) const {
  switch (type) {
  case R_DANGCSA_PCREL32:
  case R_DANGCSA_BRANCH32:
  case R_DANGCSA_REL32:
  case R_DANGCSA_REL64:
    return R_PC;
  case R_DANGCSA_64:
  case R_DANGCSA_32:
  case R_DANGCSA_IMM32:
  case R_DANGCSA_Q0:
  case R_DANGCSA_Q1:
  case R_DANGCSA_Q2:
  case R_DANGCSA_Q3:
  case R_DANGCSA_RELATIVE:
    return R_ABS;
  case R_DANGCSA_NONE:
    return R_NONE;
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unknown relocation " << type;
    return R_NONE;
  }
}

RelType dangcsa::getDynRel(RelType type) const {
  if (type == R_DANGCSA_64) {
    return R_DANGCSA_64;
  }

  return R_DANGCSA_NONE;
}

void dangcsa::relocate(uint8_t *loc, const Relocation &rel,
                       uint64_t val) const {
  switch (rel.type) {
  case R_DANGCSA_NONE:
    break;
  case R_DANGCSA_64:
  case R_DANGCSA_RELATIVE:
    write64le(loc, val);
    break;
  case R_DANGCSA_32:
    checkIntUInt(ctx, loc, val, 32, rel);
    write32le(loc, val);
    break;
  case R_DANGCSA_IMM32:
    checkIntUInt(ctx, loc, val, 32, rel);
    write32le(loc, val & 0xffffffff);
    break;
  case R_DANGCSA_PCREL32: {
    int64_t off = static_cast<int64_t>(val) + 8;
    checkInt(ctx, loc, off, 32, rel);
    write32le(loc, off & 0xffffffff);
    break;
  }
  case R_DANGCSA_BRANCH32: {
    checkAlignment(ctx, loc, val, 8, rel);
    int64_t off = static_cast<int64_t>(val) >> 3;
    checkInt(ctx, loc, off, 32, rel);
    write32le(loc, off & 0xffffffff);
    break;
  }
  case R_DANGCSA_REL32:
    checkInt(ctx, loc, static_cast<int64_t>(val), 32, rel);
    write32le(loc, val & 0xffffffff);
    break;
  case R_DANGCSA_REL64:
    write64le(loc, val);
    break;
  case R_DANGCSA_Q0:
    write16le(loc, val & 0xffff);
    break;
  case R_DANGCSA_Q1:
    write16le(loc, (val >> 16) & 0xffff);
    break;
  case R_DANGCSA_Q2:
    write16le(loc, (val >> 32) & 0xffff);
    break;
  case R_DANGCSA_Q3:
    write16le(loc, (val >> 48) & 0xffff);
    break;
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unrecognized relocation " << rel.type;
  }
}

void dangcsa::relocateAlloc(InputSection &sec, uint8_t *buf) const {
  for (const Relocation &rel : sec.relocs()) {
    if (rel.expr != R_ABS || rel.sym->isUndefWeak())
      continue;

    if (auto *d = dyn_cast<Defined>(rel.sym))
      if (!d->section)
        continue;

    Err(ctx) << getErrorLoc(ctx, buf + rel.offset) << "absolute address of "
             << rel.sym << " in an allocated section requires relocation; "
             << "pointers in initialized data are forbidden (OC Std 15.4)";
  }

  TargetInfo::relocateAlloc(sec, buf);
}

void elf::setdangcsaTargetInfo(Ctx &ctx) { ctx.target.reset(new dangcsa(ctx)); }

#define DANGCSA_EMIRU_HDR_SIZE 32
#define DANGCSA_EMIRU_MAGIC_LEN 6
#define DANGCSA_EMIRU_REV 1
#define DANGCSA_EMIRU_FLAG_PIC 0x01
#define DANGCSA_EMIRU_OFF_REV 6
#define DANGCSA_EMIRU_OFF_RING 7
#define DANGCSA_EMIRU_OFF_FLAGS 8
#define DANGCSA_EMIRU_OFF_RESERVED0 9
#define DANGCSA_EMIRU_OFF_PROG_REV 10
#define DANGCSA_EMIRU_OFF_ENTRY 12
#define DANGCSA_EMIRU_OFF_TEXT_SIZE 16
#define DANGCSA_EMIRU_OFF_DATA_SIZE 20
#define DANGCSA_EMIRU_OFF_BSS_SIZE 24
#define DANGCSA_EMIRU_OFF_RESERVED1 28

void elf::writeDangcsaEmiruHeader(Ctx &ctx, uint8_t *buf, uint64_t fileSize) {
  uint64_t minAddr = UINT64_MAX;
  uint64_t memEnd = 0;
  uint64_t entry = 0;
  uint64_t text = fileSize - DANGCSA_EMIRU_HDR_SIZE;
  uint64_t fileEnd = 0;
  uint64_t bss = 0;

  for (OutputSection *sec : ctx.outputSections) {
    if ((sec->flags & SHF_ALLOC) == 0 || sec->size == 0) {
      continue;
    }

    if (sec->type != SHT_NOBITS && sec->getLMA() < minAddr) {
      minAddr = sec->getLMA();
    }

    if (sec->addr + sec->size > memEnd) {
      memEnd = sec->addr + sec->size;
    }
  }

  if (minAddr == UINT64_MAX) {
    minAddr = 0;
  }

  if (Symbol *s = ctx.symtab->find(ctx.arg.entry)) {
    entry = s->getVA(ctx) - minAddr;
  }

  fileEnd = minAddr + text;
  bss = (memEnd > fileEnd) ? (memEnd - fileEnd) : 0;

  if (text == 0) {
    Err(ctx) << "emiru: empty text";
    return;
  }

  if (entry >= text) {
    Err(ctx) << "emiru: entry " << entry << " is past the end of text " << text;
    return;
  }

  if (text > UINT32_MAX || bss > UINT32_MAX) {
    Err(ctx) << "emiru: image does not fit the rev 1 header fields";
    return;
  }

  memcpy(buf, "EMIRU", DANGCSA_EMIRU_MAGIC_LEN);
  buf[DANGCSA_EMIRU_OFF_REV] = DANGCSA_EMIRU_REV;
  buf[DANGCSA_EMIRU_OFF_RING] = (uint8_t)ctx.arg.emiruRing;
  buf[DANGCSA_EMIRU_OFF_FLAGS] = ctx.arg.isPic ? DANGCSA_EMIRU_FLAG_PIC : 0;
  buf[DANGCSA_EMIRU_OFF_RESERVED0] = 0;
  write16le(buf + DANGCSA_EMIRU_OFF_PROG_REV, (uint16_t)ctx.arg.emiruProgRev);
  write32le(buf + DANGCSA_EMIRU_OFF_ENTRY, (uint32_t)entry);
  write32le(buf + DANGCSA_EMIRU_OFF_TEXT_SIZE, (uint32_t)text);
  write32le(buf + DANGCSA_EMIRU_OFF_DATA_SIZE, 0);
  write32le(buf + DANGCSA_EMIRU_OFF_BSS_SIZE, (uint32_t)bss);
  write32le(buf + DANGCSA_EMIRU_OFF_RESERVED1, 0);
}
