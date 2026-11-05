# LLVM core-tree patches for DANGCSA/DANGXA

Everything outside `llvm/lib/Target/dangcsa` that the DANGCSA/DANGXA backend
needs. All of it is applied by `llvm/apply_core_patches.py`, which is
idempotent and safe to re-run; this file explains what each step does and why,
and is not itself the mechanism.

```
python3 llvm/apply_core_patches.py [llvm-project root]
```

Default root is `/home/dangling/git/llvm-project_misc`. Source files that live
in this repository are symlinked into the LLVM tree rather than copied, so
editing them here is immediately visible to a rebuild. The backend directory
itself is symlinked the same way: `llvm/lib/Target/dangcsa` points at this
repository's `llvm/` directory.

The script only adds. A tree that still carries the r0.4 `misc32` payloads
(`Triple::misc32`, `EM_MISC`, `Targets/MISC.*`, `Arch/MISC.cpp`, the
`lib/Target/MISC` symlink) must have those core files restored from upstream
before the script runs; the one exception is `ELF.h`, where a stale `EM_MISC`
line and a stale `R_MISC_*` block are stripped automatically.

Two of the steps change files that CMake reads at configure time
(`clang/lib/Basic/CMakeLists.txt` and `lld/ELF/CMakeLists.txt`), so a fresh
clone needs the patcher run before the first `cmake`, and an existing build
directory needs a reconfigure after the patcher adds them.

---

## Steps

| Step | File | Change |
|------|------|--------|
| lib/Target/dangcsa link | `llvm/lib/Target/dangcsa` | symlink to this repository's `llvm/` |
| Triple.h | `llvm/include/llvm/TargetParser/Triple.h` | `dangcsa64` in `ArchType` |
| Triple.cpp | `llvm/lib/TargetParser/Triple.cpp` | seven functions, listed below |
| ELFRelocs/dangcsa.def | `llvm/include/llvm/BinaryFormat/ELFRelocs/` | symlink to `llvm/elfrelocs/dangcsa.def` |
| ELF.h | `llvm/include/llvm/BinaryFormat/ELF.h` | `EM_DANGCSA`, `ELFOSABI_DANGLING`, relocation enum |
| ELF.cpp | `llvm/lib/Object/ELF.cpp` | relocation-name table |
| ELFObjectFile.h | `llvm/include/llvm/Object/ELFObjectFile.h` | format name and arch |
| TargetDataLayout.cpp | `llvm/lib/TargetParser/TargetDataLayout.cpp` | the data layout string |
| RuntimeLibcalls.td | `llvm/include/llvm/IR/RuntimeLibcalls.td` | libcall availability |
| clang link | `clang/lib/Basic/Targets/` | symlink `dangcsa.h`, `dangcsa.cpp` |
| clang Targets.cpp | `clang/lib/Basic/Targets.cpp` | include and dispatch case |
| clang CMakeLists.txt | `clang/lib/Basic/CMakeLists.txt` | `Targets/dangcsa.cpp` |
| lld link | `lld/ELF/Arch/` | symlink to `llvm/lld/dangcsa.cpp` |
| lld Target | `lld/ELF/Target.h`, `Target.cpp` | declaration and `EM_DANGCSA` case |
| lld CMakeLists.txt | `lld/ELF/CMakeLists.txt` | `Arch/dangcsa.cpp` |
| lld EMIRU output | `lld/ELF/{Target.h,Config.h,Options.td,Driver.cpp,Writer.cpp}` | `--oformat=emiru` native EMIRU writer, see below |
| EMIRU object reader | `Magic.{h,cpp}`, `Object/{Binary.h,Binary.cpp,SymbolicFile.cpp,ObjectFile.cpp,CMakeLists.txt}`, `llvm-objdump.cpp` | `file_magic::emiru`, `ID_EMIRU`, dispatch, dumper; symlinks `llvm/emiru/EMIRU.h` and `EMIRUObjectFile.cpp`; tool dir `llvm/tools/llvm-reademiru` symlinked from `llvm/reademiru/` |
| DANGLING OS triple | `TargetParser/Triple.h`, `TargetParser/TripleName.def` | `Triple::DANGLING`, canonical name `DANGLING` with lowercase alias; `dangcsa64-DANGLING` (or `dangcsa64-dangling`) parses; clang (our `dangcsa.cpp`) defines `__DANGLING__`/`__dangling__` and drops `__ELF__` under it |

---

## Triple

`Triple::dangcsa64` is the only registered name. `dangcsa` alone is not a
triple and fails with "unable to get target". The seven `Triple.cpp` functions
are `getArchTypeName`, `getArchTypePrefix`, `parseArch`,
`getArchPointerBitWidth`, `get32BitArchVariant`, `get64BitArchVariant` and
`getDefaultFormat`. The 32-bit variant maps to `UnknownArch`; r0.5.0 has no
32-bit DANGCSA/DANGXA.

## ELF

`EM_DANGCSA` is `0xBABE`; `0xD15C` (the r0.4 `EM_MISC`) is retired.
Relocations are:

| Value | Name | Meaning |
|-------|------|---------|
| 0 | `R_DANGCSA_NONE` | |
| 1 | `R_DANGCSA_64` | absolute 64-bit word |
| 2 | `R_DANGCSA_32` | absolute 32-bit word |
| 3 | `R_DANGCSA_IMM32` | low 32 bits of `S + A` into `conf[31:0]` (sign-extended by the core) |
| 4 | `R_DANGCSA_PCREL32` | `S + A - P + 8` into `conf[31:0]` of the `add i` that follows a `cpy rd, loc` |
| 5 | `R_DANGCSA_BRANCH32` | `(S + A - P) >> 3` into `conf[31:0]` |
| 6 | `R_DANGCSA_RELATIVE` | dynamic 64-bit base-relative word |
| 7-10 | `R_DANGCSA_Q0..Q3` | 16-bit quarter `n` of `S + A` into `conf[15:0]` of a `cpy qn i` |
| 11 | `R_DANGCSA_REL32` | plain `S + A - P` in a 32-bit data word (relative lookup tables, DWARF) |
| 12 | `R_DANGCSA_REL64` | plain `S + A - P` in a 64-bit data word |

`PCREL32` carries `+8` because the `cpy rd, loc` that reads the locator sits
one instruction before the `add i` that holds the fixup: the locator value is
`P - 8`, so `rd = (P - 8) + (S + A - P + 8) = S + A`.

The relocation enum in `ELF.h` must be placed next to the other relocation
enums, immediately before the MSP430 block. `ELF.h` defines `ELF_RELOC` around
that region and `#undef`s it afterwards, so a block placed near the end of the
namespace makes `dangcsa.def` fire its own `#error "ELF_RELOC must be
defined"`. The patcher strips a misplaced block and re-anchors it rather than
assuming a clean tree.

`ELF.cpp` and `ELFObjectFile.h` are display only. Without them `llvm-objdump`
prints `Unknown (2)` and `elf64-unknown`; the relocations themselves are
unaffected.

## Data layout

`e-m:e-p:64:64-i8:8-i16:16-i32:32-i64:64-i128:128-f32:32-f64:64-n64-S64`

`i128:128` is required: clang assumes 16-byte alignment for `__int128` on a
64-bit target and aborts at startup if the layout string disagrees.

This version keeps the authoritative layout in `Triple::computeDataLayout`,
which is why clang's `resetDataLayout()` takes no argument.
`dangcsaTargetMachine` holds its own copy of the same string and the two must
match. The model is LP64: `int` 32, `long` and pointers 64, `size_t` is
`unsigned long`.

## Runtime libcalls

Adds `IsDANGCSA` alongside the other arch predicates and a
`DANGCSASystemLibrary` that holds exactly four entries:
`ldg_mem_rt_cpy`/`ldg_mem_rt_mov`/`ldg_mem_rt_fill`/`ldg_mem_rt_cmp`, provided
by LDG. No compiler-runtime entry is available on DANGCSA: 128-bit shifts and
multiplies are expanded inline by the DAG legalizer, 128-bit division,
remainder and integer/floating-point conversion by `ExpandIRInsts` (the target
sets both width limits to 64), and every `float`/`double` operation by
`dangcsaFloatExpand`, which replaces it with integer IEEE-754 code placed in
internal functions of the module being compiled. An operation none of them
covers is a compile error naming it, never an external symbol.

Libcall names are TableGen-generated per-triple availability sets. An arch with
no entry gets an empty table, `getLibcallName` returns null, and the symptom is
`basic_string: construction from null` thrown from inside `LowerCall`, which
reads as a target bug rather than missing configuration.
`llvm/test/isel_libcall.ll` covers it.

An existing tree whose `RuntimeLibcalls.td` still holds the earlier block
(`DefaultRuntimeLibcallImpls` minus the four memory calls) keeps it, because
the patcher only adds; replace that block by hand with the one above.

## clang

`llvm/clang/dangcsa.{h,cpp}` define `dangcsaTargetInfo`: LP64,
`VoidPtrBuiltinVaList`, dr0-dr15/tdr0-tdr5/loc/sp as GCC register names, and
`resetDataLayout()` with no argument. Predefined macros: `__dangcsa__`,
`__DANGCSA__`, `__dangcsa64__`, `__DANGCSA64__`, `__DLU__`.

## lld

`llvm/lld/dangcsa.cpp` is a `TargetInfo` subclass. There is no GOT, PLT or
TLS, so `getRelExpr` is two cases (`R_PC` for `PCREL32` and `BRANCH32`, `R_ABS`
for the rest) and `relocate` mirrors `dangcsaAsmBackend::applyFixup` exactly.
The assembler and the linker are independent implementations of one encoding;
a mismatch shows up only as a wrong branch at run time.

`defaultImageBase` is 0 and both page sizes are 8, so a linked image has no
segment padding to strip. `trapInstr` is the byte pattern `00 00 c0 0f`; two
copies form the word `0x0FC000000FC00000`, a `cpy` whose destination is
register 63, which raises exception `0x06` (reserved register) if ever
executed.

Use it with `llvm/dangcsa.ld`:

```
ld.lld -T llvm/dangcsa.ld --defsym __stack_top=0x00100000 -o out.elf crt0.o ...
llvm-objcopy -O binary out.elf out.img
```

## EMIRU output

`--oformat=emiru` makes lld write an EMIRU executable directly: the 32-byte
header followed by the loaded image. It rides the `--oformat binary`
machinery: `assignFileOffsetsBinary` starts file offsets at the header size,
and `writeDangcsaEmiruHeader` (in `llvm/lld/dangcsa.cpp`) backfills the header
after `writeSectionsBinary`.

Header fields: `entry` is the entry symbol's address minus the image's minimum
load address (the canonical entry is `_rst`, set by `ENTRY(_rst)` in
`llvm/dangcsa.ld`). `text_size` covers the whole file payload and `data_size`
is 0. `bss_size` is the gap between the file image's end and the highest
allocated address, which equals `__bss_end - __bss_start` under `dangcsa.ld`.
The PIC flag follows `-pie`. `ring` and `prog_rev` come from `--emiru_ring=N`
(default 0, must be 0-3) and `--emiru_prog_rev=N` (default 1). The header
stays at rev 1 with 32-bit size fields; an image whose text or bss does not fit
32 bits is refused at link time.

`--oformat=emiru` is an output format only; `-b`/`--format` (input) does not
accept it, and inputs stay ELF objects. The EMIRU container itself carries no
sections, symbols or relocations; symbolization uses the `-Map` file.

## EMIRU object reader

`identify_magic` knows the `EMIRU\0` magic, and `EMIRUObjectFile`
(`llvm/emiru/`, symlinked into `llvm/lib/Object` and
`llvm/include/llvm/Object`) makes any generic Object-library consumer open an
`.emiru` directly. The reader reports arch `dangcsa64` on its own, 8 bytes per
address, and exposes three synthetic sections named `text`, `data` and `bss`
built from the header sizes. `llvm-objdump -h/-d/--private-headers` all work;
the private-headers dumper prints the decoded EMIRU header. `llvm-reademiru`
(`llvm/reademiru/`, symlinked as a tool directory that LLVM's CMake
auto-discovers) prints the full parsed header, load-image size, trailing-byte
check and validity verdict. The reader validates per OC Std 15.3: magic, rev
1, ring <= 3, nonzero text, entry inside text, buffer at least header plus
payload.
