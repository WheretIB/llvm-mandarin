add_llvm_library(LLVMMandarinDesc
  MandarinMCTargetDesc.cpp
  MandarinMCAsmInfo.cpp
  )

add_dependencies(LLVMMandarinDesc MandarinCommonTableGen)
