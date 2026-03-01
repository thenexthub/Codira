set(LLVM_LINK_COMPONENTS
  nativecodegen
  native
  orcodeit
  support
)

# Main API shared library.
add_mlir_upstream_c_api_library(MLIRCAPIExecutionEngine
  ExecutionEngine.cpp

  LINK_LIBS PUBLIC
  MLIRBuiltinToLLVMIRTranslation
  MLIRExecutionEngine
  MLIRLLVMToLLVMIRTranslation
)
