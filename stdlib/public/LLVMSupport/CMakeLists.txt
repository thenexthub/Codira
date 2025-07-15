add_language_target_library(languageLLVMSupport OBJECT_LIBRARY
    ErrorHandling.cpp
    Hashing.cpp
    MemAlloc.cpp
    SmallPtrSet.cpp
    SmallVector.cpp
    StringRef.cpp
  INSTALL_IN_COMPONENT
    never_install)
