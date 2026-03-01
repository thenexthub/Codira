add_llvm_component_library(LLVMFrontendOpenACC
  ACC.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/Frontend
  ${LLVM_MAIN_INCLUDE_DIR}/toolchain/Frontend/OpenACC

  DEPENDS
  acc_gen
)

target_link_libraries(LLVMFrontendOpenACC LLVMSupport LLVMFrontendDirective)

