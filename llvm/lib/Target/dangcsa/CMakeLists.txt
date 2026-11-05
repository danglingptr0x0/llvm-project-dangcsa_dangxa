add_llvm_component_group(dangcsa)

set(LLVM_TARGET_DEFINITIONS dangcsa.td)

tablegen(LLVM dangcsaGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM dangcsaGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM dangcsaGenCallingConv.inc -gen-callingconv)
tablegen(LLVM dangcsaGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM dangcsaGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM dangcsaGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM dangcsaGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM dangcsaGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM dangcsaGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(dangcsaCommonTableGen)

add_llvm_target(dangcsaCodeGen
  dangcsaAsmPrinter.cpp
  dangcsaFloatExpand.cpp
  dangcsaFrameLowering.cpp
  dangcsaISelDAGToDAG.cpp
  dangcsaISelLowering.cpp
  dangcsaInstrInfo.cpp
  dangcsaMCInstLower.cpp
  dangcsaRegisterInfo.cpp
  dangcsaSubtarget.cpp
  dangcsaTargetMachine.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
  dangcsaDesc
  dangcsaInfo
  SelectionDAG
  Support
  Target
  TargetParser

  ADD_TO_COMPONENT
  dangcsa
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
