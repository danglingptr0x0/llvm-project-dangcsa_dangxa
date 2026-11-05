add_llvm_component_library(LLVMdangcsaInfo
  dangcsaTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support
  TargetParser

  ADD_TO_COMPONENT
  dangcsa
  )
