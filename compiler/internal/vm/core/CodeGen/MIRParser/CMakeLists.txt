add_llvm_component_library(LLVMMIRParser
  MILexer.cpp
  MIParser.cpp
  MIRParser.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/CodeGen/MIRParser

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  AsmParser
  BinaryFormat
  CodeGen
  CodeGenTypes
  Core
  MC
  Support
  Target
  )
