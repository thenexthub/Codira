add_llvm_component_library(LLVMInstCombine
  InstructionCombining.cpp
  InstCombineAddSub.cpp
  InstCombineAtomicRMW.cpp
  InstCombineAndOrXor.cpp
  InstCombineCalls.cpp
  InstCombineCasts.cpp
  InstCombineCompares.cpp
  InstCombineLoadStoreAlloca.cpp
  InstCombineMulDivRem.cpp
  InstCombineNegator.cpp
  InstCombinePHI.cpp
  InstCombineSelect.cpp
  InstCombineShifts.cpp
  InstCombineSimplifyDemanded.cpp
  InstCombineVectorOps.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/Transforms/InstCombine

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  Support
  TransformUtils
  )
