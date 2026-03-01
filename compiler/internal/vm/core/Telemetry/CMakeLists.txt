add_llvm_component_library(LLVMTelemetry
  Telemetry.cpp

  ADDITIONAL_HEADER_DIRS
  "${LLVM_MAIN_INCLUDE_DIR}/toolchain/Telemetry"

  LINK_COMPONENTS
    Support
)
