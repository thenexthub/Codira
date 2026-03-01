add_llvm_component_library(LLVMMCODEIT
  MCODEIT.cpp

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Core
  ExecutionEngine
  Object
  RuntimeDyld
  Support
  Target
  )
