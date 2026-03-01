add_llvm_component_library(LLVMAggressiveInstCombine
  AggressiveInstCombine.cpp
  TruncInstCombine.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/Transforms/AggressiveInstCombine

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  Support
  TransformUtils
  )
