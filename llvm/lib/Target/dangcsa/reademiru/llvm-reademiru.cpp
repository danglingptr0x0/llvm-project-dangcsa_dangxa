//===- llvm-reademiru.cpp - EMIRU header reader ---------------------------===//
//
// Copyright (c) 2026 Ongakken Corporation OU
// All rights reserved.
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/EMIRU.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::object;

static cl::OptionCategory ReadEmiruCat("llvm-reademiru options");

static cl::list<std::string> InputFiles(cl::Positional,
                                        cl::desc("<emiru files>"),
                                        cl::OneOrMore, cl::cat(ReadEmiruCat));

static int dumpOne(StringRef Path) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> BufOrErr = MemoryBuffer::getFile(Path);
  if (!BufOrErr) {
    WithColor::error(errs(), "llvm-reademiru")
        << Path << ": " << BufOrErr.getError().message() << "\n";
    return 1;
  }

  Expected<std::unique_ptr<EMIRUObjectFile>> ObjOrErr =
      EMIRUObjectFile::create((*BufOrErr)->getMemBufferRef());
  if (!ObjOrErr) {
    WithColor::error(errs(), "llvm-reademiru")
        << Path << ": " << toString(ObjOrErr.takeError()) << "\n";
    return 1;
  }

  EMIRUObjectFile &Obj = **ObjOrErr;
  uint64_t FileSize = (*BufOrErr)->getBufferSize();
  uint64_t Payload = (uint64_t)Obj.getTextSize() + Obj.getDataSize();
  uint64_t Trailing = FileSize - 32 - Payload;

  outs() << Path << ":\n";
  outs() << "  Magic:      EMIRU\n";
  outs() << "  Rev:        1\n";
  outs() << "  Ring:       " << (unsigned)Obj.getRing() << "\n";
  outs() << "  Flags:      0x";
  outs().write_hex(Obj.getEmiruFlags());
  if (Obj.getEmiruFlags() & 1)
    outs() << " (PIC)";
  outs() << "\n";
  outs() << "  ProgRev:    " << Obj.getProgRev() << "\n";
  outs() << "  Entry:      0x";
  outs().write_hex(Obj.getEntry());
  outs() << "\n";
  outs() << "  TextSize:   " << Obj.getTextSize() << "\n";
  outs() << "  DataSize:   " << Obj.getDataSize() << "\n";
  outs() << "  BssSize:    " << Obj.getBssSize() << "\n";
  outs() << "  FileSize:   " << FileSize << "\n";
  outs() << "  LoadImage:  " << Payload + Obj.getBssSize()
         << " bytes in memory\n";
  if (Trailing != 0)
    outs() << "  Trailing:   " << Trailing
           << " bytes past the declared payload\n";
  outs() << "  Valid:      yes\n";
  return 0;
}

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);
  cl::HideUnrelatedOptions({&ReadEmiruCat});
  cl::ParseCommandLineOptions(argc, argv, "EMIRU header reader\n");

  int Ret = 0;
  for (const std::string &Path : InputFiles)
    Ret |= dumpOne(Path);
  return Ret;
}
