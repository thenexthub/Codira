add_llvm_component_library(LLVMBitstreamReader
  BitstreamReader.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/Bitcode
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/Bitstream

  LINK_COMPONENTS
  Support
  )
