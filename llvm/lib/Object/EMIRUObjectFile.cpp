//===- EMIRUObjectFile.cpp - EMIRU executable format ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/EMIRU.h"
#include "llvm/Object/Error.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;

static const unsigned HdrSize = 32;
static const unsigned OffRing = 7;
static const unsigned OffFlags = 8;
static const unsigned OffProgRev = 10;
static const unsigned OffEntry = 12;
static const unsigned OffTextSize = 16;
static const unsigned OffDataSize = 20;
static const unsigned OffBssSize = 24;
static const unsigned SecText = 0;
static const unsigned SecData = 1;
static const unsigned SecBss = 2;
static const unsigned SecCount = 3;

EMIRUObjectFile::EMIRUObjectFile(MemoryBufferRef Object)
    : ObjectFile(ID_EMIRU, Object) {}

uint32_t EMIRUObjectFile::readWord(unsigned Offset) const {
  return read32le(base() + Offset);
}

uint16_t EMIRUObjectFile::readHalf(unsigned Offset) const {
  return read16le(base() + Offset);
}

uint8_t EMIRUObjectFile::getRing() const { return base()[OffRing]; }
uint8_t EMIRUObjectFile::getEmiruFlags() const { return base()[OffFlags]; }
uint16_t EMIRUObjectFile::getProgRev() const { return readHalf(OffProgRev); }
uint32_t EMIRUObjectFile::getEntry() const { return readWord(OffEntry); }
uint32_t EMIRUObjectFile::getTextSize() const { return readWord(OffTextSize); }
uint32_t EMIRUObjectFile::getDataSize() const { return readWord(OffDataSize); }
uint32_t EMIRUObjectFile::getBssSize() const { return readWord(OffBssSize); }

Expected<std::unique_ptr<EMIRUObjectFile>>
EMIRUObjectFile::create(MemoryBufferRef Object) {
  StringRef Data = Object.getBuffer();
  if (Data.size() < HdrSize)
    return errorCodeToError(object_error::unexpected_eof);
  if (!Data.starts_with(StringRef("EMIRU\0", 6)))
    return errorCodeToError(object_error::parse_failed);

  std::unique_ptr<EMIRUObjectFile> Obj(new EMIRUObjectFile(Object));

  if (Data[6] != 1)
    return createStringError(object_error::parse_failed,
                             "unsupported EMIRU rev %u", (unsigned)Data[6]);
  if (Obj->getRing() > 3)
    return createStringError(object_error::parse_failed, "invalid ring %u",
                             (unsigned)Obj->getRing());

  uint64_t Text = Obj->getTextSize();
  uint64_t DataSz = Obj->getDataSize();
  if (Text == 0)
    return createStringError(object_error::parse_failed, "empty text");
  if (Obj->getEntry() >= Text)
    return createStringError(object_error::parse_failed,
                             "entry past the end of text");
  if (Text + DataSz < Text || Data.size() < HdrSize + Text + DataSz)
    return errorCodeToError(object_error::unexpected_eof);

  return std::move(Obj);
}

void EMIRUObjectFile::moveSymbolNext(DataRefImpl &Symb) const { Symb.d.a++; }

Expected<uint32_t> EMIRUObjectFile::getSymbolFlags(DataRefImpl) const {
  llvm_unreachable("EMIRU has no symbols");
}

basic_symbol_iterator EMIRUObjectFile::symbol_begin() const {
  return basic_symbol_iterator(BasicSymbolRef(DataRefImpl(), this));
}

basic_symbol_iterator EMIRUObjectFile::symbol_end() const {
  return basic_symbol_iterator(BasicSymbolRef(DataRefImpl(), this));
}

Expected<StringRef> EMIRUObjectFile::getSymbolName(DataRefImpl) const {
  llvm_unreachable("EMIRU has no symbols");
}

Expected<uint64_t> EMIRUObjectFile::getSymbolAddress(DataRefImpl) const {
  llvm_unreachable("EMIRU has no symbols");
}

uint64_t EMIRUObjectFile::getSymbolValueImpl(DataRefImpl) const {
  llvm_unreachable("EMIRU has no symbols");
}

Expected<SymbolRef::Type> EMIRUObjectFile::getSymbolType(DataRefImpl) const {
  llvm_unreachable("EMIRU has no symbols");
}

Expected<section_iterator>
EMIRUObjectFile::getSymbolSection(DataRefImpl) const {
  llvm_unreachable("EMIRU has no symbols");
}

uint64_t EMIRUObjectFile::getCommonSymbolSizeImpl(DataRefImpl) const {
  llvm_unreachable("EMIRU has no symbols");
}

void EMIRUObjectFile::moveSectionNext(DataRefImpl &Sec) const { Sec.d.a++; }

Expected<StringRef> EMIRUObjectFile::getSectionName(DataRefImpl Sec) const {
  switch (Sec.d.a) {
  case SecText:
    return StringRef("text");
  case SecData:
    return StringRef("data");
  case SecBss:
    return StringRef("bss");
  }
  llvm_unreachable("bad section index");
}

uint64_t EMIRUObjectFile::getSectionAddress(DataRefImpl Sec) const {
  switch (Sec.d.a) {
  case SecText:
    return 0;
  case SecData:
    return getTextSize();
  case SecBss:
    return (uint64_t)getTextSize() + getDataSize();
  }
  llvm_unreachable("bad section index");
}

uint64_t EMIRUObjectFile::getSectionIndex(DataRefImpl Sec) const {
  return Sec.d.a;
}

uint64_t EMIRUObjectFile::getSectionSize(DataRefImpl Sec) const {
  switch (Sec.d.a) {
  case SecText:
    return getTextSize();
  case SecData:
    return getDataSize();
  case SecBss:
    return getBssSize();
  }
  llvm_unreachable("bad section index");
}

Expected<ArrayRef<uint8_t>>
EMIRUObjectFile::getSectionContents(DataRefImpl Sec) const {
  switch (Sec.d.a) {
  case SecText:
    return ArrayRef<uint8_t>(base() + HdrSize, getTextSize());
  case SecData:
    return ArrayRef<uint8_t>(base() + HdrSize + getTextSize(), getDataSize());
  case SecBss:
    return ArrayRef<uint8_t>();
  }
  llvm_unreachable("bad section index");
}

uint64_t EMIRUObjectFile::getSectionAlignment(DataRefImpl) const { return 8; }

bool EMIRUObjectFile::isSectionCompressed(DataRefImpl) const { return false; }

bool EMIRUObjectFile::isSectionText(DataRefImpl Sec) const {
  return Sec.d.a == SecText;
}

bool EMIRUObjectFile::isSectionData(DataRefImpl Sec) const {
  return Sec.d.a == SecData;
}

bool EMIRUObjectFile::isSectionBSS(DataRefImpl Sec) const {
  return Sec.d.a == SecBss;
}

bool EMIRUObjectFile::isSectionVirtual(DataRefImpl Sec) const {
  return Sec.d.a == SecBss;
}

relocation_iterator EMIRUObjectFile::section_rel_begin(DataRefImpl) const {
  return relocation_iterator(RelocationRef(DataRefImpl(), this));
}

relocation_iterator EMIRUObjectFile::section_rel_end(DataRefImpl) const {
  return relocation_iterator(RelocationRef(DataRefImpl(), this));
}

section_iterator EMIRUObjectFile::section_begin() const {
  DataRefImpl Sec;
  Sec.d.a = SecText;
  return section_iterator(SectionRef(Sec, this));
}

section_iterator EMIRUObjectFile::section_end() const {
  DataRefImpl Sec;
  Sec.d.a = SecCount;
  return section_iterator(SectionRef(Sec, this));
}

void EMIRUObjectFile::moveRelocationNext(DataRefImpl &) const {
  llvm_unreachable("EMIRU has no relocations");
}

uint64_t EMIRUObjectFile::getRelocationOffset(DataRefImpl) const {
  llvm_unreachable("EMIRU has no relocations");
}

symbol_iterator EMIRUObjectFile::getRelocationSymbol(DataRefImpl) const {
  llvm_unreachable("EMIRU has no relocations");
}

uint64_t EMIRUObjectFile::getRelocationType(DataRefImpl) const {
  llvm_unreachable("EMIRU has no relocations");
}

void EMIRUObjectFile::getRelocationTypeName(DataRefImpl,
                                            SmallVectorImpl<char> &) const {
  llvm_unreachable("EMIRU has no relocations");
}

uint8_t EMIRUObjectFile::getBytesInAddress() const { return 8; }

StringRef EMIRUObjectFile::getFileFormatName() const { return "emiru"; }

Triple::ArchType EMIRUObjectFile::getArch() const { return Triple::dangcsa64; }

Expected<SubtargetFeatures> EMIRUObjectFile::getFeatures() const {
  return SubtargetFeatures();
}

Expected<uint64_t> EMIRUObjectFile::getStartAddress() const {
  return getEntry();
}

bool EMIRUObjectFile::isRelocatableObject() const { return false; }

Expected<std::unique_ptr<ObjectFile>>
llvm::object::createEMIRUObjectFile(MemoryBufferRef Object) {
  return EMIRUObjectFile::create(Object);
}
