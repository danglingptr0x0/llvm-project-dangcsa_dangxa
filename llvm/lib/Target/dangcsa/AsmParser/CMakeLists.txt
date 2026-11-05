add_llvm_component_library(LLVMdangcsaAsmParser
  dangcsaAsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  dangcsaDesc
  dangcsaInfo
  Support

  ADD_TO_COMPONENT
  dangcsa
  )
