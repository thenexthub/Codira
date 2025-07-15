add_library(languageLLVMSupport OBJECT
  ErrorHandling.cpp
  Hashing.cpp
  MemAlloc.cpp
  SmallPtrSet.cpp
  SmallVector.cpp
  StringRef.cpp)
target_compile_options(languageLLVMSupport
  PRIVATE
    $<$<BOOL:${CodiraCore_HAS_ASL}>:-DLANGUAGE_STDLIB_HAS_ASL>)

if(NOT BUILD_SHARED_LIBS)
  install(TARGETS languageLLVMSupport
    EXPORT CodiraCoreTargets
    COMPONENT CodiraCore_runtime)
endif()
