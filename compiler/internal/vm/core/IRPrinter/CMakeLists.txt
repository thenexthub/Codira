add_llvm_component_library(LLVMIRPrinter
  IRPrintingPasses.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/IRPrinter

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  Support
  )
