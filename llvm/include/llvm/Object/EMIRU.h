//===- EMIRU.h - EMIRU executable format ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// The EMIRU executable: a 32-byte header, then text, then data. The loader
// zero-initializes bss_size bytes after data. The container carries no
// sections, symbols or relocations; the reader exposes text, data and bss as
// synthetic sections so generic tools can walk the image.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OBJECT_EMIRU_H
#define LLVM_OBJECT_EMIRU_H

#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/Error.h"
#include "llvm/TargetParser/SubtargetFeature.h"
#include "llvm/TargetParser/Triple.h"

namespace llvm {
namespace object {

class EMIRUObjectFile : public ObjectFile {
public:
  static Expected<std::unique_ptr<EMIRUObjectFile>>
  create(MemoryBufferRef Object);

  static bool classof(const Binary *V) { return V->getType() == ID_EMIRU; }

  uint8_t getRing() const;
  uint8_t getEmiruFlags() const;
  uint16_t getProgRev() const;
  uint32_t getEntry() const;
  uint32_t getTextSize() const;
  uint32_t getDataSize() const;
  uint32_t getBssSize() const;

  // SymbolicFile
  void moveSymbolNext(DataRefImpl &Symb) const override;
  Expected<uint32_t> getSymbolFlags(DataRefImpl Symb) const override;
  basic_symbol_iterator symbol_begin() const override;
  basic_symbol_iterator symbol_end() const override;
  bool is64Bit() const override { return false; }

  // ObjectFile symbols
  Expected<StringRef> getSymbolName(DataRefImpl Symb) const override;
  Expected<uint64_t> getSymbolAddress(DataRefImpl Symb) const override;
  uint64_t getSymbolValueImpl(DataRefImpl Symb) const override;
  Expected<SymbolRef::Type> getSymbolType(DataRefImpl Symb) const override;
  Expected<section_iterator> getSymbolSection(DataRefImpl Symb) const override;
  uint64_t getCommonSymbolSizeImpl(DataRefImpl Symb) const override;

  // ObjectFile sections
  void moveSectionNext(DataRefImpl &Sec) const override;
  Expected<StringRef> getSectionName(DataRefImpl Sec) const override;
  uint64_t getSectionAddress(DataRefImpl Sec) const override;
  uint64_t getSectionIndex(DataRefImpl Sec) const override;
  uint64_t getSectionSize(DataRefImpl Sec) const override;
  Expected<ArrayRef<uint8_t>>
  getSectionContents(DataRefImpl Sec) const override;
  uint64_t getSectionAlignment(DataRefImpl Sec) const override;
  bool isSectionCompressed(DataRefImpl Sec) const override;
  bool isSectionText(DataRefImpl Sec) const override;
  bool isSectionData(DataRefImpl Sec) const override;
  bool isSectionBSS(DataRefImpl Sec) const override;
  bool isSectionVirtual(DataRefImpl Sec) const override;
  relocation_iterator section_rel_begin(DataRefImpl Sec) const override;
  relocation_iterator section_rel_end(DataRefImpl Sec) const override;
  section_iterator section_begin() const override;
  section_iterator section_end() const override;

  // ObjectFile relocations (the container has none)
  void moveRelocationNext(DataRefImpl &Rel) const override;
  uint64_t getRelocationOffset(DataRefImpl Rel) const override;
  symbol_iterator getRelocationSymbol(DataRefImpl Rel) const override;
  uint64_t getRelocationType(DataRefImpl Rel) const override;
  void getRelocationTypeName(DataRefImpl Rel,
                             SmallVectorImpl<char> &Result) const override;

  uint8_t getBytesInAddress() const override;
  StringRef getFileFormatName() const override;
  Triple::ArchType getArch() const override;
  Expected<SubtargetFeatures> getFeatures() const override;
  Expected<uint64_t> getStartAddress() const override;
  bool isRelocatableObject() const override;

private:
  EMIRUObjectFile(MemoryBufferRef Object);

  uint32_t readWord(unsigned Offset) const;
  uint16_t readHalf(unsigned Offset) const;
};

Expected<std::unique_ptr<ObjectFile>>
createEMIRUObjectFile(MemoryBufferRef Object);

} // namespace object
} // namespace llvm

#endif
