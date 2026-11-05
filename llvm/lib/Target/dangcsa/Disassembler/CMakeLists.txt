add_llvm_component_library(LLVMdangcsaDisassembler
  dangcsaDisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  dangcsaDesc
  dangcsaInfo
  Support

  ADD_TO_COMPONENT
  dangcsa
  )
