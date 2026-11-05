set(LLVM_LINK_COMPONENTS
  Object
  Support
  TargetParser
  )

add_llvm_tool(llvm-reademiru
  llvm-reademiru.cpp
  )
