add_llvm_component_library(LLVMdangcsaDesc
  dangcsaAsmBackend.cpp
  dangcsaELFObjectWriter.cpp
  dangcsaInstPrinter.cpp
  dangcsaMCAsmInfo.cpp
  dangcsaMCCodeEmitter.cpp
  dangcsaMCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  dangcsaInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  dangcsa
  )
