
add_language_host_library(languageLLVMPasses STATIC
  LLVMCodiraAA.cpp
  LLVMCodiraRCIdentity.cpp
  LLVMARCOpts.cpp
  LLVMARCContract.cpp
  LLVMInlineTree.cpp
  LLVMMergeFunctions.cpp
  LLVMEmitAsyncEntryReturnMetadata.cpp

  TOOLCHAIN_LINK_COMPONENTS
  analysis
  targetparser
  )
target_link_libraries(languageLLVMPasses PRIVATE
  languageDemangling)

set_language_toolchain_is_available(languageLLVMPasses)
