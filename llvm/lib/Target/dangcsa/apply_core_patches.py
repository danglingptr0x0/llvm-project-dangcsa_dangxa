import os
import sys

DEFAULT_ROOT = "/home/dangling/git/llvm-project_misc"

ARCH_NAME = "dangcsa64"
ARCH_PREFIX = "dangcsa"

ARCH_DECL = "    dangcsa64,\n"
ARCH_LAST = "    LastArchType = dangcsa64\n"

NAME_CASE = "  case dangcsa64:      return \"dangcsa64\";\n"
PREFIX_CASE = "  case dangcsa64:      return \"dangcsa\";\n"
PARSE_CASE = "                .Case(\"dangcsa64\", Triple::dangcsa64)\n"
WIDTH_CASE = "  case llvm::Triple::dangcsa64:\n    return 64;\n"
VARIANT32_CASE = "  case Triple::dangcsa64:\n    T.setArch(UnknownArch);\n    break;\n"
VARIANT64_CASE = "  case Triple::dangcsa64:\n    break;\n"
FORMAT_CASE = "  case Triple::dangcsa64:\n    return Triple::ELF;\n"

EM_LINE = "  EM_DANGCSA = 0xBABE,\n"
EM_OLD_LINE = "  EM_MISC = 0xD15C,\n"

RELOC_HEAD = "// ELF Relocation types for DANGCSA\n"

RELOC_BLOCK = (
    RELOC_HEAD
    + "enum {\n"
    "#include \"ELFRelocs/dangcsa.def\"\n"
    "};\n"
    "\n"
)

RELOC_ANCHOR = "// ELF Relocation types for MSP430\n"

RELOC_DEF_NAME = "dangcsa.def"
RELOC_DEF_MARK = "ELFRelocs/dangcsa.def"
RELOC_OLD_HEAD = "// ELF Relocation types for MISC\n"
RELOC_OLD_MARKS = ("ELFRelocs/MISC.def", "R_MISC_NONE")

IND2 = "\x20\x20"
IND4 = "\x20\x20\x20\x20"

CLANG_INCL_ANCHOR = "#include \"Targets/Lanai.h\"\n"
CLANG_INCL_LINE = "#include \"Targets/dangcsa.h\"\n"
CLANG_CASE_ANCHOR = IND2 + "case llvm::Triple::lanai:\n" + IND4 + "return std::make_unique<LanaiTargetInfo>(Triple, Opts);\n"
CLANG_CASE_LINE = "\n" + IND2 + "case llvm::Triple::dangcsa64:\n" + IND4 + "return std::make_unique<dangcsaTargetInfo>(Triple, Opts);\n"
CLANG_CMAKE_ANCHOR = IND2 + "Targets/M68k.cpp\n"
CLANG_CMAKE_LINE = IND2 + "Targets/dangcsa.cpp\n"
CLANG_SRC_NAMES = ("dangcsa.h", "dangcsa.cpp")

DATALAYOUT_STR = "e-m:e-p:64:64-i8:8-i16:16-i32:32-i64:64-i128:128-f32:32-f64:64-n64-S64"
DATALAYOUT_ANCHOR = IND2 + "case Triple::lanai:\n" + IND4 + "return computeLanaiDataLayout();\n"
DATALAYOUT_LINE = IND2 + "case Triple::dangcsa64:\n" + IND4 + "return \"" + DATALAYOUT_STR + "\";\n"

LIBCALL_PRED_ANCHOR = "def IsMSP430 : LibcallPredicate<[{TT.getArch() == Triple::msp430}]>;\n"
LIBCALL_PRED_LINE = "def IsDANGCSA : LibcallPredicate<[{TT.getArch() == Triple::dangcsa64}]>;\n"

LIBCALL_LIB_ANCHOR = "// NVPTX Runtime Libcalls\n"
LIBCALL_LIB_MARK = "def isDANGCSA"
LIBCALL_LIB_LINE = (
    "// DANGCSA Runtime Libcalls\n"
    "//===----------------------------------------------------------------------===//\n"
    "\n"
    "def isDANGCSA : RuntimeLibcallAvailability<(all_of IsDANGCSA)>;\n"
    "\n"
    "def ldg_mem_rt_cpy : RuntimeLibcallImpl<MEMCPY>;\n"
    "def ldg_mem_rt_mov : RuntimeLibcallImpl<MEMMOVE>;\n"
    "def ldg_mem_rt_fill : RuntimeLibcallImpl<MEMSET>;\n"
    "def ldg_mem_rt_cmp : RuntimeLibcallImpl<MEMCMP>;\n"
    "\n"
    "def DANGCSASystemLibrary\n"
    + IND4 + ": SystemRuntimeLibrary<isDANGCSA,\n"
    + IND4 + "(add ldg_mem_rt_cpy, ldg_mem_rt_mov, ldg_mem_rt_fill, ldg_mem_rt_cmp)>;\n"
    "\n"
    "//===----------------------------------------------------------------------===//\n"
)

ELF_CPP_ANCHOR = IND2 + "case ELF::EM_LANAI:\n"
ELF_CPP_LINE = (
    IND2 + "case ELF::EM_DANGCSA:\n"
    + IND4 + "switch (Type) {\n"
    + "#include \"llvm/BinaryFormat/ELFRelocs/dangcsa.def\"\n"
    + IND4 + "default:\n"
    + IND4 + IND2 + "break;\n"
    + IND4 + "}\n"
    + IND4 + "break;\n"
)

ELF_FORMAT_ANCHOR = IND4 + "case ELF::EM_S390:\n" + IND4 + IND2 + "return \"elf64-s390\";\n"
ELF_FORMAT_MISPLACED = IND4 + "case ELF::EM_MSP430:\n" + IND4 + IND2 + "return \"elf32-msp430\";\n"
ELF_FORMAT_LINE = IND4 + "case ELF::EM_DANGCSA:\n" + IND4 + IND2 + "return \"elf64-dangcsa\";\n"

ELF_ARCH_ANCHOR = IND2 + "case ELF::EM_AVR:\n" + IND4 + "return Triple::avr;\n"
ELF_ARCH_LINE = IND2 + "case ELF::EM_DANGCSA:\n" + IND4 + "return Triple::dangcsa64;\n"

OSABI_ANCHOR = IND2 + "ELFOSABI_STANDALONE = 255,"
OSABI_LINE = IND2 + "ELFOSABI_DANGLING = 0x44, // DANGLING\n"

READOBJ_OSABI_ANCHOR = IND4 + "{{\"Standalone\", \"Standalone App\"}, ELF::ELFOSABI_STANDALONE}};\n"
READOBJ_OSABI_LINE = IND4 + "{{\"DANGLING\", \"DANGLING\"}, ELF::ELFOSABI_DANGLING},\n"

READOBJ_ENUM_GAP_WIDTH = 10
READOBJ_ENUM_GAP = " " * READOBJ_ENUM_GAP_WIDTH
READOBJ_MACHINE_ANCHOR = IND2 + "ENUM_ENT(EM_NONE," + READOBJ_ENUM_GAP + "\"None\"),\n"
READOBJ_MACHINE_LINE = IND2 + "ENUM_ENT(EM_DANGCSA," + " " * 7 + "\"DANGCSA\"),\n"

LLD_SRC_NAME = "dangcsa.cpp"
LLD_DECL_ANCHOR = "void setMSP430TargetInfo(Ctx &);\n"
LLD_DECL_LINE = "void setdangcsaTargetInfo(Ctx &);\n"
LLD_CASE_ANCHOR = IND2 + "case EM_MSP430:\n" + IND4 + "return setMSP430TargetInfo(ctx);\n"
LLD_CASE_LINE = IND2 + "case EM_DANGCSA:\n" + IND4 + "return setdangcsaTargetInfo(ctx);\n"
LLD_CMAKE_ANCHOR = IND2 + "Arch/MSP430.cpp\n"
LLD_CMAKE_LINE = IND2 + "Arch/dangcsa.cpp\n"

EMIRU_DECL_ANCHOR = "void setdangcsaTargetInfo(Ctx &);\n"
EMIRU_DECL_LINE = "void writeDangcsaEmiruHeader(Ctx &, uint8_t *, uint64_t);\n"

EMIRU_CONFIG_ANCHOR = IND2 + "bool oFormatBinary;\n"
EMIRU_CONFIG_LINES = (
    IND2 + "bool oFormatEmiru = false;\n"
    + IND2 + "unsigned emiruRing = 0;\n"
    + IND2 + "unsigned emiruProgRev = 1;\n"
)

EMIRU_OPT_ANCHOR = (
    "defm oformat: EEq<\"oformat\", \"Specify the binary format for the output object file\">,\n"
    + IND2 + "MetaVarName<\"[elf,binary]\">;\n"
)
EMIRU_OPT_LINES = (
    "defm oformat: EEq<\"oformat\", \"Specify the binary format for the output object file\">,\n"
    + IND2 + "MetaVarName<\"[elf,binary,emiru]\">;\n"
    "\n"
    "defm emiru_ring: EEq<\"emiru_ring\", \"Ring field of the EMIRU header (0-3, default 0)\">,\n"
    + IND2 + "MetaVarName<\"<ring>\">;\n"
    "\n"
    "defm emiru_prog_rev: EEq<\"emiru_prog_rev\", \"prog_rev field of the EMIRU header (default 1)\">,\n"
    + IND2 + "MetaVarName<\"<rev>\">;\n"
)

EMIRU_DRIVER_FMT_SIG = "static bool isOutputFormatBinary("
EMIRU_DRIVER_FMT_ANCHOR = IND2 + "if (s == \"binary\")\n" + IND4 + "return true;\n"
EMIRU_DRIVER_FMT_LINES = (
    IND2 + "if (s == \"emiru\") {\n"
    + IND4 + "ctx.arg.oFormatEmiru = true;\n"
    + IND4 + "return true;\n"
    + IND2 + "}\n"
)

EMIRU_DRIVER_ARG_ANCHOR = IND2 + "ctx.arg.oFormatBinary = isOutputFormatBinary(ctx, args);\n"
EMIRU_DRIVER_ARG_LINES = (
    IND2 + "ctx.arg.emiruRing = args::getInteger(args, OPT_emiru_ring, 0);\n"
    + IND2 + "ctx.arg.emiruProgRev = args::getInteger(args, OPT_emiru_prog_rev, 1);\n"
    + IND2 + "if (ctx.arg.emiruRing > 3)\n"
    + IND4 + "ErrAlways(ctx) << \"--emiru_ring must be 0-3\";\n"
)

EMIRU_WRITER_OFF_ANCHOR = (
    IND2 + "// Sections are laid out at LMA minus minAddr.\n"
    + IND2 + "fileSize = 0;\n"
)
EMIRU_WRITER_OFF_LINES = (
    IND2 + "// Sections are laid out at LMA minus minAddr, after any raw container header.\n"
    + IND2 + "uint64_t hdrSize = ctx.arg.oFormatEmiru ? 32 : 0;\n"
    + IND2 + "fileSize = hdrSize;\n"
)
EMIRU_WRITER_OFF_FIX_OLD = IND4 + IND2 + "sec->offset -= minAddr;\n"
EMIRU_WRITER_OFF_FIX_NEW = IND4 + IND2 + "sec->offset = sec->offset - minAddr + hdrSize;\n"

EMIRU_WRITER_CALL_ANCHOR = IND4 + IND2 + "writeSectionsBinary();\n"
EMIRU_WRITER_CALL_LINES = (
    IND4 + IND2 + "if (ctx.arg.oFormatEmiru)\n"
    + IND4 + IND4 + "writeDangcsaEmiruHeader(ctx, ctx.bufferStart, fileSize);\n"
)

CONFIG_IX_ANCHOR = """elseif (LLVM_NATIVE_ARCH MATCHES "loongarch")
  set(LLVM_NATIVE_ARCH LoongArch)
"""

CONFIG_IX_LINE = """elseif (LLVM_NATIVE_ARCH MATCHES "dangcsa")
  set(LLVM_NATIVE_ARCH dangcsa)
"""

EMIRU_OBJ_SRC_NAMES = ("EMIRU.h", "EMIRUObjectFile.cpp")

EMIRU_MAGIC_H_ANCHOR = IND4 + "spirv_object," + (" " * 14) + "///< A binary SPIR-V file\n"
EMIRU_MAGIC_H_LINE = IND4 + "emiru,\n"

EMIRU_MAGIC_CPP_ANCHOR = IND2 + "case 'B':\n"
EMIRU_MAGIC_CPP_LINES = (
    IND2 + "case 'E':\n"
    + IND4 + "if (startswith(Magic, StringRef(\"EMIRU\\0\", 6)))\n"
    + IND4 + IND2 + "return file_magic::emiru;\n"
    + IND4 + "break;\n"
)

EMIRU_BINARY_H_ANCHOR = IND4 + "ID_DXContainer,\n"
EMIRU_BINARY_H_LINE = IND4 + "ID_EMIRU,\n"

EMIRU_BINARY_CPP_ANCHOR = IND2 + "case file_magic::dxcontainer_object:\n" + IND4 + "return ObjectFile::createSymbolicFile(Buffer, Type, Context, InitContent);\n"
EMIRU_BINARY_CPP_LINE = IND2 + "case file_magic::emiru:\n"

EMIRU_SYMFILE_CREATE_ANCHOR = IND2 + "case file_magic::dxcontainer_object:\n" + IND4 + "return ObjectFile::createObjectFile(Object, Type, InitContent);\n"
EMIRU_SYMFILE_IS_ANCHOR = IND2 + "case file_magic::dxcontainer_object:\n" + IND4 + "return true;\n"

EMIRU_OBJFILE_INCL_ANCHOR = "#include \"llvm/Object/DXContainer.h\"\n"
EMIRU_OBJFILE_INCL_LINE = "#include \"llvm/Object/EMIRU.h\"\n"
EMIRU_OBJFILE_CASE_ANCHOR = IND2 + "case file_magic::goff_object:\n" + IND4 + "return createGOFFObjectFile(Object);\n"
EMIRU_OBJFILE_CASE_LINES = (
    IND2 + "case file_magic::emiru:\n"
    + IND4 + "return createEMIRUObjectFile(Object);\n"
)

EMIRU_OBJ_CMAKE_ANCHOR = IND2 + "ELF.cpp\n"
EMIRU_OBJ_CMAKE_LINE = IND2 + "EMIRUObjectFile.cpp\n"

EMIRU_SYMFILE_IS_MARK = "file_magic::emiru:\n" + IND4 + "return true;"

IND3 = " " * 3

EMIRU_OBJDUMP_INCL_ANCHOR = "#include \"llvm/Object/ELFObjectFile.h\"\n"
EMIRU_OBJDUMP_INCL_LINE = "#include \"llvm/Object/EMIRU.h\"\n"
EMIRU_OBJDUMP_DUMPER_ANCHOR = "static Expected<std::unique_ptr<Dumper>> createDumper(const ObjectFile &Obj) {\n"
EMIRU_OBJDUMP_DUMPER_BLOCK = (
    "namespace {\n"
    "class EMIRUDumper : public Dumper {\n"
    + IND2 + "const EMIRUObjectFile &Obj;\n"
    "\n"
    "public:\n"
    + IND2 + "EMIRUDumper(const EMIRUObjectFile &O) : Dumper(O), Obj(O) {}\n"
    + IND2 + "void printPrivateHeaders() override {\n"
    + IND4 + "OS << \"EMIRU header: rev 1 ring \" << (unsigned)Obj.getRing()\n"
    + IND4 + IND3 + "<< \" flags 0x\" << Twine::utohexstr(Obj.getEmiruFlags())\n"
    + IND4 + IND3 + "<< \" prog_rev \" << Obj.getProgRev() << \" entry 0x\"\n"
    + IND4 + IND3 + "<< Twine::utohexstr(Obj.getEntry()) << \" text \" << Obj.getTextSize()\n"
    + IND4 + IND3 + "<< \" data \" << Obj.getDataSize() << \" bss \" << Obj.getBssSize()\n"
    + IND4 + IND3 + "<< \"\\n\";\n"
    + IND2 + "}\n"
    "};\n"
    "} // namespace\n"
    "\n"
)
EMIRU_OBJDUMP_DISPATCH_LINES = (
    IND2 + "if (const auto *O = dyn_cast<EMIRUObjectFile>(&Obj))\n"
    + IND4 + "return std::make_unique<EMIRUDumper>(*O);\n"
)

DANGLING_OS_ANCHOR = IND2 + "enum OSType {\n" + IND4 + "UnknownOS,\n\n"
DANGLING_OS_LINE = IND4 + "DANGLING,\n"
DANGLING_OS_DEF_ANCHOR = "TRIPLE_OS(Darwin, \"darwin\", \"Darwin\")\n"
DANGLING_OS_DEF_LINE = (
    "TRIPLE_OS(DANGLING, \"DANGLING\", \"Generic\")\n"
    "TRIPLE_OS_ALIAS(DANGLING, \"dangling\")\n"
)
DANGLING_ELF_OLD = (
    IND2 + "if (TI.getTriple().isOSBinFormatELF())\n"
    + IND4 + "Builder.defineMacro(\"__ELF__\");\n"
)
DANGLING_ELF_NEW = (
    IND2 + "if (TI.getTriple().isOSBinFormatELF() &&\n"
    + IND2 + IND4 + "TI.getTriple().getOS() != llvm::Triple::DANGLING)\n"
    + IND4 + "Builder.defineMacro(\"__ELF__\");\n"
)


def read(path):
    with open(path, "r", encoding="utf-8") as fh:
        return fh.read()


def write(path, text):
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(text)


def here_dir():
    return os.path.dirname(os.path.abspath(__file__))


def link_one(src, dst):
    name = os.path.basename(dst)

    if not os.path.exists(src):
        return name + ": missing file"

    if os.path.islink(dst) and os.path.realpath(dst) == os.path.realpath(src):
        return name + ": already applied"

    if os.path.islink(dst) or os.path.exists(dst):
        if os.path.isdir(dst) and not os.path.islink(dst):
            return name + ": destination is a real directory; remove it by hand"
        os.remove(dst)

    os.symlink(src, dst)
    return name + ": linked"


def find_function(text, signature):
    start = text.find(signature)
    if start < 0:
        return -1, -1
    end = text.find("\n}\n", start)
    if end < 0:
        return -1, -1
    return start, end


def insert_after_in_function(text, signature, anchor, payload, marker):
    if marker in text:
        return text, "already applied"

    start, end = find_function(text, signature)
    if start < 0:
        return text, "function not found: " + signature

    body = text[start:end]
    idx = body.find(anchor)
    if idx < 0:
        return text, "anchor not found in " + signature + ": " + anchor.strip()

    cut = start + idx + len(anchor)
    return text[:cut] + payload + text[cut:], "patched"


def insert_before(text, anchor, payload, marker, label):
    if marker in text:
        return text, label + ": already applied"

    idx = text.find(anchor)
    if idx < 0:
        return text, label + ": anchor not found"

    return text[:idx] + payload + text[idx:], label + ": patched"


def insert_after(text, anchor, payload, marker, label):
    if marker in text:
        return text, label + ": already applied"

    idx = text.find(anchor)
    if idx < 0:
        return text, label + ": anchor not found"

    cut = idx + len(anchor)
    return text[:cut] + payload + text[cut:], label + ": patched"


def patch_target_link(root):
    src = here_dir()
    dst = os.path.join(root, "llvm/lib/Target", ARCH_PREFIX)

    if not os.path.isdir(os.path.join(root, "llvm/lib/Target")):
        return "missing file: " + os.path.join(root, "llvm/lib/Target")

    return link_one(src, dst)


def patch_triple_h(root):
    path = os.path.join(root, "llvm/include/llvm/TargetParser/Triple.h")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)
    if ARCH_NAME in text:
        return "already applied"

    marker = None
    for line in text.splitlines(True):
        if line.strip().startswith("LastArchType ="):
            marker = line
            break

    if marker is None:
        return "anchor not found: LastArchType"

    text = text.replace(marker, ARCH_DECL + ARCH_LAST, 1)
    write(path, text)
    return "patched"


def patch_triple_cpp(root):
    path = os.path.join(root, "llvm/lib/TargetParser/Triple.cpp")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)
    results = []

    steps = [
        ("StringRef Triple::getArchTypeName(", "switch (Kind) {\n", NAME_CASE,
         "return \"dangcsa64\""),
        ("StringRef Triple::getArchTypePrefix(", "switch (Kind) {\n",
         PREFIX_CASE, "return \"dangcsa\""),
        ("parseArch(StringRef ArchName)",
         "StringSwitch<Triple::ArchType>(ArchName)\n",
         PARSE_CASE, "\"dangcsa64\", Triple::dangcsa64"),
        ("unsigned Triple::getArchPointerBitWidth(", "switch (Arch) {\n",
         WIDTH_CASE, "llvm::Triple::dangcsa64"),
        ("Triple Triple::get32BitArchVariant(", "switch (getArch()) {\n",
         VARIANT32_CASE, "case Triple::dangcsa64:\n    T.setArch(UnknownArch);"),
        ("Triple Triple::get64BitArchVariant(", "switch (getArch()) {\n",
         VARIANT64_CASE, "case Triple::dangcsa64:\n    break;"),
        ("getDefaultFormat(const Triple", "switch (T.getArch()) {\n",
         FORMAT_CASE, "case Triple::dangcsa64:" + NL4 + "return Triple::ELF;"),
        ("bool Triple::isLittleEndian() const", "switch (getArch()) {\n",
         ENDIAN_CASE, "isLittleEndian"),
    ]

    for signature, anchor, payload, marker in steps:
        text, status = insert_after_in_function(text, signature, anchor,
                                                payload, marker)
        results.append(signature.split("(")[0].split()[-1] + ": " + status)

    write(path, text)
    return "; ".join(results)


def reloc_block_strip(text, head, marks):
    marker = -1

    for mark in marks:
        marker = text.find(mark)
        if marker >= 0:
            break

    if marker < 0:
        return text

    start = text.rfind("enum {", 0, marker)
    end = text.find("};", marker)

    if start < 0 or end < 0:
        return text

    cut = end + len("};\n")

    if text.startswith(head, start - len(head)):
        start -= len(head)

    while text.startswith("\n", cut):
        cut += 1

    return text[:start] + text[cut:]


def patch_elf_h(root):
    path = os.path.join(root, "llvm/include/llvm/BinaryFormat/ELF.h")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)
    results = []

    if EM_OLD_LINE in text:
        text = text.replace(EM_OLD_LINE, "", 1)
        results.append("EM_MISC: removed")

    if "EM_DANGCSA" in text:
        results.append("EM_DANGCSA: already applied")
    else:
        anchor = "  EM_NONE = 0,"
        idx = text.find(anchor)
        if idx < 0:
            results.append("EM_DANGCSA: anchor not found")
        else:
            eol = text.find("\n", idx) + 1
            text = text[:eol] + EM_LINE + text[eol:]
            results.append("EM_DANGCSA: patched")

    if "ELFOSABI_DANGLING" in text:
        results.append("ELFOSABI_DANGLING: already applied")
    else:
        idx = text.find(OSABI_ANCHOR)
        if idx < 0:
            results.append("ELFOSABI_DANGLING: anchor not found")
        else:
            text = text[:idx] + OSABI_LINE + text[idx:]
            results.append("ELFOSABI_DANGLING: patched")

    stripped = reloc_block_strip(text, RELOC_OLD_HEAD, RELOC_OLD_MARKS)
    if stripped != text:
        text = stripped
        results.append("MISC relocs: removed")

    placed = (RELOC_BLOCK + RELOC_ANCHOR) in text

    if placed:
        results.append("relocs: already applied")
    else:
        text = reloc_block_strip(text, RELOC_HEAD, (RELOC_DEF_MARK, "R_DANGCSA_NONE"))
        idx = text.find(RELOC_ANCHOR)
        if idx < 0:
            results.append("relocs: anchor not found")
        else:
            text = text[:idx] + RELOC_BLOCK + text[idx:]
            results.append("relocs: placed")

    write(path, text)
    return "; ".join(results)


def patch_clang_link(root):
    src_dir = os.path.join(here_dir(), "clang")
    dst_dir = os.path.join(root, "clang/lib/Basic/Targets")

    if not os.path.isdir(dst_dir):
        return "missing file: " + dst_dir

    results = []

    for name in CLANG_SRC_NAMES:
        results.append(link_one(os.path.join(src_dir, name), os.path.join(dst_dir, name)))

    return "; ".join(results)


def patch_clang_targets(root):
    path = os.path.join(root, "clang/lib/Basic/Targets.cpp")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)
    results = []

    if "Targets/dangcsa.h" in text:
        results.append("include: already applied")
    else:
        idx = text.find(CLANG_INCL_ANCHOR)
        if idx < 0:
            results.append("include: anchor not found")
        else:
            cut = idx + len(CLANG_INCL_ANCHOR)
            text = text[:cut] + CLANG_INCL_LINE + text[cut:]
            results.append("include: patched")

    if "dangcsaTargetInfo>(Triple, Opts)" in text:
        results.append("case: already applied")
    else:
        idx = text.find(CLANG_CASE_ANCHOR)
        if idx < 0:
            results.append("case: anchor not found")
        else:
            cut = idx + len(CLANG_CASE_ANCHOR)
            text = text[:cut] + CLANG_CASE_LINE + text[cut:]
            results.append("case: patched")

    write(path, text)
    return "; ".join(results)


def patch_clang_cmake(root):
    path = os.path.join(root, "clang/lib/Basic/CMakeLists.txt")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)

    if "Targets/dangcsa.cpp" in text:
        return "already applied"

    idx = text.find(CLANG_CMAKE_ANCHOR)
    if idx < 0:
        return "anchor not found: Targets/M68k.cpp"

    text = text[:idx] + CLANG_CMAKE_LINE + text[idx:]
    write(path, text)
    return "patched"


def patch_config_ix(root):
    path = os.path.join(root, "llvm/cmake/config-ix.cmake")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)

    if "set(LLVM_NATIVE_ARCH dangcsa)" in text:
        return "already applied"

    idx = text.find(CONFIG_IX_ANCHOR)
    if idx < 0:
        return "anchor not found: LLVM_NATIVE_ARCH LoongArch"

    cut = idx + len(CONFIG_IX_ANCHOR)
    text = text[:cut] + CONFIG_IX_LINE + text[cut:]
    write(path, text)
    return "patched"


def patch_data_layout(root):
    path = os.path.join(root, "llvm/lib/TargetParser/TargetDataLayout.cpp")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)

    if DATALAYOUT_LINE in text:
        return "already applied"

    if "Triple::dangcsa64" in text:
        start = text.find(IND2 + "case Triple::dangcsa64:\n")
        end = text.find("\n", text.find("return \"", start)) + 1
        if start < 0 or end <= 0:
            return "stale layout: could not isolate"
        text = text[:start] + DATALAYOUT_LINE + text[end:]
        write(path, text)
        return "stale layout replaced"

    idx = text.find(DATALAYOUT_ANCHOR)
    if idx < 0:
        return "anchor not found: Triple::lanai"

    cut = idx + len(DATALAYOUT_ANCHOR)
    text = text[:cut] + DATALAYOUT_LINE + text[cut:]
    write(path, text)
    return "patched"


def patch_runtime_libcalls(root):
    path = os.path.join(root, "llvm/include/llvm/IR/RuntimeLibcalls.td")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)
    results = []

    if "IsDANGCSA" in text:
        results.append("predicate: already applied")
    else:
        idx = text.find(LIBCALL_PRED_ANCHOR)
        if idx < 0:
            results.append("predicate: anchor not found")
        else:
            cut = idx + len(LIBCALL_PRED_ANCHOR)
            text = text[:cut] + LIBCALL_PRED_LINE + text[cut:]
            results.append("predicate: patched")

    if LIBCALL_LIB_MARK in text:
        results.append("library: already applied")
    else:
        idx = text.find(LIBCALL_LIB_ANCHOR)
        if idx < 0:
            results.append("library: anchor not found")
        else:
            text = text[:idx] + LIBCALL_LIB_LINE + text[idx:]
            results.append("library: patched")

    write(path, text)
    return "; ".join(results)


def patch_reloc_def(root):
    src = os.path.join(here_dir(), "elfrelocs", RELOC_DEF_NAME)
    dst_dir = os.path.join(root, "llvm/include/llvm/BinaryFormat/ELFRelocs")

    if not os.path.isdir(dst_dir):
        return "missing file: " + dst_dir

    return link_one(src, os.path.join(dst_dir, RELOC_DEF_NAME))


def patch_elf_cpp(root):
    path = os.path.join(root, "llvm/lib/Object/ELF.cpp")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)

    if RELOC_DEF_MARK in text:
        return "already applied"

    idx = text.find(ELF_CPP_ANCHOR)
    if idx < 0:
        return "anchor not found: EM_LANAI"

    text = text[:idx] + ELF_CPP_LINE + text[idx:]
    write(path, text)
    return "patched"


def patch_elf_object_file(root):
    path = os.path.join(root, "llvm/include/llvm/Object/ELFObjectFile.h")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)
    results = []

    if (ELF_FORMAT_LINE + ELF_FORMAT_MISPLACED) in text:
        text = text.replace(ELF_FORMAT_LINE + ELF_FORMAT_MISPLACED, ELF_FORMAT_MISPLACED, 1)
        results.append("format: misplaced case removed")

    if "elf64-dangcsa" in text:
        results.append("format: already applied")
    else:
        idx = text.find(ELF_FORMAT_ANCHOR)
        if idx < 0:
            results.append("format: anchor not found")
        else:
            text = text[:idx] + ELF_FORMAT_LINE + text[idx:]
            results.append("format: patched")

    if "Triple::dangcsa64" in text:
        results.append("arch: already applied")
    else:
        idx = text.find(ELF_ARCH_ANCHOR)
        if idx < 0:
            results.append("arch: anchor not found")
        else:
            cut = idx + len(ELF_ARCH_ANCHOR)
            text = text[:cut] + ELF_ARCH_LINE + text[cut:]
            results.append("arch: patched")

    write(path, text)
    return "; ".join(results)


def patch_readobj_machine(root):
    path = os.path.join(root, "llvm/tools/llvm-readobj/ELFDumper.cpp")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)

    results = []

    if "EM_DANGCSA" in text:
        results.append("machine: already applied")
    else:
        idx = text.find(READOBJ_MACHINE_ANCHOR)
        if idx < 0:
            results.append("machine: anchor not found")
        else:
            cut = idx + len(READOBJ_MACHINE_ANCHOR)
            text = text[:cut] + READOBJ_MACHINE_LINE + text[cut:]
            results.append("machine: patched")

    if "ELFOSABI_DANGLING" in text:
        results.append("osabi: already applied")
    else:
        idx = text.find(READOBJ_OSABI_ANCHOR)
        if idx < 0:
            results.append("osabi: anchor not found")
        else:
            text = text[:idx] + READOBJ_OSABI_LINE + text[idx:]
            results.append("osabi: patched")

    write(path, text)
    return "; ".join(results)


def patch_lld_link(root):
    src = os.path.join(here_dir(), "lld", LLD_SRC_NAME)
    dst_dir = os.path.join(root, "lld/ELF/Arch")

    if not os.path.isdir(dst_dir):
        return "missing file: " + dst_dir

    return link_one(src, os.path.join(dst_dir, LLD_SRC_NAME))


def patch_lld_target(root):
    head = os.path.join(root, "lld/ELF/Target.h")
    body = os.path.join(root, "lld/ELF/Target.cpp")
    results = []

    if not os.path.exists(head):
        return "missing file: " + head

    if not os.path.exists(body):
        return "missing file: " + body

    text = read(head)

    if "setdangcsaTargetInfo" in text:
        results.append("decl: already applied")
    else:
        idx = text.find(LLD_DECL_ANCHOR)
        if idx < 0:
            results.append("decl: anchor not found")
        else:
            text = text[:idx] + LLD_DECL_LINE + text[idx:]
            results.append("decl: patched")

    write(head, text)

    text = read(body)

    if "setdangcsaTargetInfo" in text:
        results.append("case: already applied")
    else:
        idx = text.find(LLD_CASE_ANCHOR)
        if idx < 0:
            results.append("case: anchor not found")
        else:
            text = text[:idx] + LLD_CASE_LINE + text[idx:]
            results.append("case: patched")

    write(body, text)
    return "; ".join(results)


def patch_lld_cmake(root):
    path = os.path.join(root, "lld/ELF/CMakeLists.txt")
    if not os.path.exists(path):
        return "missing file: " + path

    text = read(path)

    if "Arch/dangcsa.cpp" in text:
        return "already applied"

    idx = text.find(LLD_CMAKE_ANCHOR)
    if idx < 0:
        return "anchor not found: Arch/MSP430.cpp"

    text = text[:idx] + LLD_CMAKE_LINE + text[idx:]
    write(path, text)
    return "patched"


ENDIAN_CASE = IND2 + "case Triple::dangcsa64:\n"
OLD_GAP9 = " " * 9
OLD_GAP10 = " " * 10
OLD_GAP16 = " " * 16
NL4 = "\n" + IND4
NL2 = "\n" + IND2

OLD_TEXT_PAYLOADS = (
    ("llvm/include/llvm/TargetParser/Triple.h", IND4 + "misc32,\n"),
    ("llvm/lib/TargetParser/Triple.cpp", IND2 + "case misc32:" + OLD_GAP9 + "return \"misc32\";\n"),
    ("llvm/lib/TargetParser/Triple.cpp", IND2 + "case misc32:" + OLD_GAP9 + "return \"misc\";\n"),
    ("llvm/lib/TargetParser/Triple.cpp", OLD_GAP16 + ".Case(\"misc32\", Triple::misc32)\n"),
    ("llvm/lib/TargetParser/Triple.cpp", IND2 + "case llvm::Triple::misc32:" + NL4 + "return 32;\n"),
    ("llvm/lib/TargetParser/Triple.cpp", IND2 + "case Triple::misc32:" + NL4 + "T.setArch(UnknownArch);" + NL4 + "break;\n"),
    ("llvm/lib/TargetParser/Triple.cpp", IND2 + "case Triple::misc32:" + NL4 + "break;\n"),
    ("llvm/lib/TargetParser/Triple.cpp", IND2 + "case Triple::misc32:" + NL4 + "return Triple::ELF;\n"),
    ("llvm/lib/TargetParser/Triple.cpp", IND2 + "case Triple::misc32:\n"),
    ("llvm/lib/TargetParser/Triple.cpp", IND2 + "case llvm::Triple::misc32:\n"),
    ("llvm/lib/TargetParser/TargetDataLayout.cpp", IND2 + "case Triple::misc32:" + NL4 + "return \"e-m:e-p:32:32-i8:8-i16:16-i32:32-i64:32-f64:32-n32-S32\";\n"),
    ("llvm/include/llvm/IR/RuntimeLibcalls.td", "def IsMISC : LibcallPredicate<[{TT.getArch() == Triple::misc32}]>;\n"),
    ("llvm/include/llvm/IR/RuntimeLibcalls.td",
     "// MISC Runtime Libcalls\n"
     "//===----------------------------------------------------------------------===//\n"
     "\n"
     "def isMISC : RuntimeLibcallAvailability<(all_of IsMISC)>;\n"
     "\n"
     "def ldg_mem_rt_cpy : RuntimeLibcallImpl<MEMCPY>;\n"
     "def ldg_mem_rt_mov : RuntimeLibcallImpl<MEMMOVE>;\n"
     "def ldg_mem_rt_fill : RuntimeLibcallImpl<MEMSET>;\n"
     "def ldg_mem_rt_cmp : RuntimeLibcallImpl<MEMCMP>;\n"
     "\n"
     "def MISCSystemLibrary\n"
     + IND4 + ": SystemRuntimeLibrary<isMISC,\n"
     + IND4 + "(add (sub DefaultRuntimeLibcallImpls, memcpy, memmove, memset, memcmp),\n"
     + IND4 + "ldg_mem_rt_cpy, ldg_mem_rt_mov, ldg_mem_rt_fill, ldg_mem_rt_cmp)>;\n"
     "\n"
     "//===----------------------------------------------------------------------===//\n"),
    ("llvm/include/llvm/IR/RuntimeLibcalls.td",
     "// MISC Runtime Libcalls\n"
     "//===----------------------------------------------------------------------===//\n"
     "\n"
     "def isMISC : RuntimeLibcallAvailability<(all_of IsMISC)>;\n"
     "\n"
     "def MISCSystemLibrary\n"
     + IND4 + ": SystemRuntimeLibrary<isMISC, (add DefaultRuntimeLibcallImpls)>;\n"
     "\n"
     "//===----------------------------------------------------------------------===//\n"),
    ("llvm/lib/Object/ELF.cpp",
     IND2 + "case ELF::EM_MISC:\n"
     + IND4 + "switch (Type) {\n"
     + "#include \"llvm/BinaryFormat/ELFRelocs/MISC.def\"\n"
     + IND4 + "default:\n"
     + IND4 + IND2 + "break;\n"
     + IND4 + "}\n"
     + IND4 + "break;\n"),
    ("llvm/include/llvm/Object/ELFObjectFile.h", IND4 + "case ELF::EM_MISC:\n" + IND4 + IND2 + "return \"elf32-misc\";\n"),
    ("llvm/include/llvm/Object/ELFObjectFile.h", IND2 + "case ELF::EM_MISC:\n" + IND4 + "return Triple::misc32;\n"),
    ("llvm/tools/llvm-readobj/ELFDumper.cpp", IND2 + "ENUM_ENT(EM_MISC," + OLD_GAP10 + "\"MISC\"),\n"),
    ("llvm/cmake/config-ix.cmake", "elseif (LLVM_NATIVE_ARCH MATCHES \"misc\")" + NL2 + "set(LLVM_NATIVE_ARCH MISC)\n"),
    ("clang/lib/Basic/Targets.cpp", "#include \"Targets/MISC.h\"\n"),
    ("clang/lib/Basic/Targets.cpp", NL2 + "case llvm::Triple::misc32:" + NL4 + "return std::make_unique<MISCTargetInfo>(Triple, Opts);\n"),
    ("clang/lib/Basic/CMakeLists.txt", IND2 + "Targets/MISC.cpp\n"),
    ("lld/ELF/Target.h", "void setMISCTargetInfo(Ctx &);\n"),
    ("lld/ELF/Target.h", "void writeMiscEmiruHeader(Ctx &, uint8_t *, uint64_t);\n"),
    ("lld/ELF/Target.cpp", IND2 + "case EM_MISC:" + NL4 + "return setMISCTargetInfo(ctx);\n"),
    ("lld/ELF/CMakeLists.txt", IND2 + "Arch/MISC.cpp\n"),
    ("lld/ELF/Writer.cpp", IND4 + IND2 + "if (ctx.arg.oFormatEmiru)\n" + IND4 + IND4 + "writeMiscEmiruHeader(ctx, ctx.bufferStart, fileSize);\n"),
    ("lld/ELF/Writer.cpp", IND4 + IND4 + IND4 + "if (ctx.arg.oFormatEmiru)\n" + IND4 + IND4 + IND4 + IND2 + "writeMiscEmiruHeader(ctx, ctx.bufferStart, fileSize);\n"),
)

OLD_LINKS = (
    "llvm/lib/Target/MISC",
    "llvm/include/llvm/BinaryFormat/ELFRelocs/MISC.def",
    "clang/lib/Basic/Targets/MISC.h",
    "clang/lib/Basic/Targets/MISC.cpp",
    "lld/ELF/Arch/MISC.cpp",
)


def strip_old_payloads(root):
    results = []

    for rel in OLD_LINKS:
        path = os.path.join(root, rel)
        if os.path.islink(path):
            os.remove(path)
            results.append(os.path.basename(rel) + ": unlinked")

    for rel, payload in OLD_TEXT_PAYLOADS:
        path = os.path.join(root, rel)
        if not os.path.exists(path):
            continue

        text = read(path)
        if payload not in text:
            continue

        write(path, text.replace(payload, ""))
        results.append(os.path.basename(rel) + ": stripped " + payload.strip().splitlines()[0])

    if not results:
        return "nothing to strip"

    return "; ".join(results)


def patch_dangling_os(root):
    results = []

    head = os.path.join(root, "llvm/include/llvm/TargetParser/Triple.h")
    if not os.path.exists(head):
        return "missing file: " + head

    text = read(head)
    text, status = insert_after(text, DANGLING_OS_ANCHOR, DANGLING_OS_LINE,
                                "DANGLING,", "ostype")
    results.append(status)
    write(head, text)

    names = os.path.join(root, "llvm/include/llvm/TargetParser/TripleName.def")
    if not os.path.exists(names):
        return "missing file: " + names

    text = read(names)
    text, status = insert_before(text, DANGLING_OS_DEF_ANCHOR,
                                 DANGLING_OS_DEF_LINE, "DANGLING", "name row")
    results.append(status)
    write(names, text)

    prep = os.path.join(root, "clang/lib/Frontend/InitPreprocessor.cpp")
    if not os.path.exists(prep):
        return "missing file: " + prep

    text = read(prep)
    if "Triple::DANGLING" in text:
        results.append("elf define: already applied")
    elif DANGLING_ELF_OLD not in text:
        results.append("elf define: anchor not found")
    else:
        text = text.replace(DANGLING_ELF_OLD, DANGLING_ELF_NEW, 1)
        write(prep, text)
        results.append("elf define: patched")

    return "; ".join(results)


def patch_emiru_object(root):
    here = here_dir()
    results = []

    tool_dir = os.path.join(root, "llvm/tools/llvm-reademiru")
    if not os.path.isdir(tool_dir):
        os.makedirs(tool_dir)

    links = (
        (os.path.join(here, "emiru", "EMIRU.h"),
         os.path.join(root, "llvm/include/llvm/Object/EMIRU.h")),
        (os.path.join(here, "emiru", "EMIRUObjectFile.cpp"),
         os.path.join(root, "llvm/lib/Object/EMIRUObjectFile.cpp")),
        (os.path.join(here, "reademiru", "llvm-reademiru.cpp"),
         os.path.join(tool_dir, "llvm-reademiru.cpp")),
        (os.path.join(here, "reademiru", "CMakeLists.txt"),
         os.path.join(tool_dir, "CMakeLists.txt")),
    )

    for src, dst in links:
        results.append(link_one(src, dst))

    steps = (
        ("llvm/include/llvm/BinaryFormat/Magic.h", "insert_after",
         EMIRU_MAGIC_H_ANCHOR, EMIRU_MAGIC_H_LINE, "emiru,", "magic enum"),
        ("llvm/lib/BinaryFormat/Magic.cpp", "insert_before",
         EMIRU_MAGIC_CPP_ANCHOR, EMIRU_MAGIC_CPP_LINES, "file_magic::emiru",
         "magic sniff"),
        ("llvm/include/llvm/Object/Binary.h", "insert_after",
         EMIRU_BINARY_H_ANCHOR, EMIRU_BINARY_H_LINE, "ID_EMIRU", "binary id"),
        ("llvm/lib/Object/Binary.cpp", "insert_before",
         EMIRU_BINARY_CPP_ANCHOR, EMIRU_BINARY_CPP_LINE, "file_magic::emiru",
         "binary case"),
        ("llvm/lib/Object/SymbolicFile.cpp", "insert_before",
         EMIRU_SYMFILE_CREATE_ANCHOR, EMIRU_BINARY_CPP_LINE,
         "file_magic::emiru", "symfile create"),
        ("llvm/lib/Object/SymbolicFile.cpp", "insert_before",
         EMIRU_SYMFILE_IS_ANCHOR, EMIRU_BINARY_CPP_LINE, EMIRU_SYMFILE_IS_MARK,
         "symfile is"),
        ("llvm/lib/Object/ObjectFile.cpp", "insert_after",
         EMIRU_OBJFILE_INCL_ANCHOR, EMIRU_OBJFILE_INCL_LINE, "Object/EMIRU.h",
         "objfile include"),
        ("llvm/lib/Object/ObjectFile.cpp", "insert_after",
         EMIRU_OBJFILE_CASE_ANCHOR, EMIRU_OBJFILE_CASE_LINES,
         "createEMIRUObjectFile(Object)", "objfile case"),
        ("llvm/lib/Object/CMakeLists.txt", "insert_after",
         EMIRU_OBJ_CMAKE_ANCHOR, EMIRU_OBJ_CMAKE_LINE, "EMIRUObjectFile.cpp",
         "object cmake"),
        ("llvm/tools/llvm-objdump/llvm-objdump.cpp", "insert_after",
         EMIRU_OBJDUMP_INCL_ANCHOR, EMIRU_OBJDUMP_INCL_LINE, "Object/EMIRU.h",
         "objdump include"),
        ("llvm/tools/llvm-objdump/llvm-objdump.cpp", "insert_before",
         EMIRU_OBJDUMP_DUMPER_ANCHOR, EMIRU_OBJDUMP_DUMPER_BLOCK,
         "EMIRUDumper", "objdump dumper"),
        ("llvm/tools/llvm-objdump/llvm-objdump.cpp", "insert_after",
         EMIRU_OBJDUMP_DUMPER_ANCHOR, EMIRU_OBJDUMP_DISPATCH_LINES,
         "make_unique<EMIRUDumper>", "objdump dispatch"),
    )

    for rel, mode, anchor, payload, marker, label in steps:
        path = os.path.join(root, rel)
        if not os.path.exists(path):
            results.append(label + ": missing file")
            continue

        text = read(path)

        if mode == "insert_after":
            text, status = insert_after(text, anchor, payload, marker, label)
        else:
            text, status = insert_before(text, anchor, payload, marker, label)

        results.append(status)
        write(path, text)

    return "; ".join(results)


def patch_lld_emiru(root):
    results = []

    head = os.path.join(root, "lld/ELF/Target.h")
    if not os.path.exists(head):
        return "missing file: " + head

    text = read(head)
    text, status = insert_after(text, EMIRU_DECL_ANCHOR, EMIRU_DECL_LINE,
                                "writeDangcsaEmiruHeader", "decl")
    results.append(status)
    write(head, text)

    conf = os.path.join(root, "lld/ELF/Config.h")
    if not os.path.exists(conf):
        return "missing file: " + conf

    text = read(conf)
    text, status = insert_after(text, EMIRU_CONFIG_ANCHOR, EMIRU_CONFIG_LINES,
                                "oFormatEmiru", "config")
    results.append(status)
    write(conf, text)

    opts = os.path.join(root, "lld/ELF/Options.td")
    if not os.path.exists(opts):
        return "missing file: " + opts

    text = read(opts)
    if "emiru_ring" in text:
        results.append("options: already applied")
    elif EMIRU_OPT_ANCHOR not in text:
        results.append("options: anchor not found")
    else:
        text = text.replace(EMIRU_OPT_ANCHOR, EMIRU_OPT_LINES, 1)
        results.append("options: patched")
    write(opts, text)

    driver = os.path.join(root, "lld/ELF/Driver.cpp")
    if not os.path.exists(driver):
        return "missing file: " + driver

    text = read(driver)
    if "s == \"emiru\"" in text:
        results.append("driver format: already applied")
    else:
        text, status = insert_after_in_function(text, EMIRU_DRIVER_FMT_SIG,
                                                EMIRU_DRIVER_FMT_ANCHOR,
                                                EMIRU_DRIVER_FMT_LINES,
                                                "s == \"emiru\"")
        results.append("driver format: " + status)
    text, status = insert_after(text, EMIRU_DRIVER_ARG_ANCHOR,
                                EMIRU_DRIVER_ARG_LINES, "OPT_emiru_ring",
                                "driver args")
    results.append(status)
    write(driver, text)

    writer = os.path.join(root, "lld/ELF/Writer.cpp")
    if not os.path.exists(writer):
        return "missing file: " + writer

    text = read(writer)
    if "hdrSize" in text:
        results.append("writer offsets: already applied")
    elif EMIRU_WRITER_OFF_ANCHOR not in text or EMIRU_WRITER_OFF_FIX_OLD not in text:
        results.append("writer offsets: anchor not found")
    else:
        text = text.replace(EMIRU_WRITER_OFF_ANCHOR, EMIRU_WRITER_OFF_LINES, 1)
        text = text.replace(EMIRU_WRITER_OFF_FIX_OLD, EMIRU_WRITER_OFF_FIX_NEW, 1)
        results.append("writer offsets: patched")
    text, status = insert_after(text, EMIRU_WRITER_CALL_ANCHOR,
                                EMIRU_WRITER_CALL_LINES,
                                "writeDangcsaEmiruHeader(ctx, ctx.bufferStart",
                                "writer call")
    results.append(status)
    write(writer, text)

    return "; ".join(results)


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_ROOT

    if not os.path.isdir(root):
        sys.stderr.write("llvm root not found: " + root + "\n")
        return 2

    outcomes = [
        ("r0.4 misc32 payloads", strip_old_payloads(root)),
        ("lib/Target/dangcsa link", patch_target_link(root)),
        ("Triple.h", patch_triple_h(root)),
        ("Triple.cpp", patch_triple_cpp(root)),
        ("ELFRelocs/dangcsa.def", patch_reloc_def(root)),
        ("ELF.h", patch_elf_h(root)),
        ("ELF.cpp", patch_elf_cpp(root)),
        ("ELFObjectFile.h", patch_elf_object_file(root)),
        ("llvm-readobj ELFDumper.cpp", patch_readobj_machine(root)),
        ("TargetDataLayout.cpp", patch_data_layout(root)),
        ("config-ix.cmake", patch_config_ix(root)),
        ("RuntimeLibcalls.td", patch_runtime_libcalls(root)),
        ("clang link", patch_clang_link(root)),
        ("clang Targets.cpp", patch_clang_targets(root)),
        ("clang CMakeLists.txt", patch_clang_cmake(root)),
        ("lld link", patch_lld_link(root)),
        ("lld Target", patch_lld_target(root)),
        ("lld CMakeLists.txt", patch_lld_cmake(root)),
        ("lld EMIRU output", patch_lld_emiru(root)),
        ("EMIRU object reader", patch_emiru_object(root)),
        ("DANGLING OS triple", patch_dangling_os(root)),
    ]

    failed = 0
    for name, status in outcomes:
        sys.stdout.write(name + " -> " + status + "\n")
        if "not found" in status or "missing" in status or "by hand" in status:
            failed += 1

    if failed:
        sys.stderr.write("%d patch step(s) failed\n" % failed)
        return 1

    return 0


sys.exit(main())
