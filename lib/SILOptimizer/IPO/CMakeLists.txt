target_sources(languageSILOptimizer PRIVATE
  CapturePropagation.cpp
  ClosureSpecializer.cpp
  CrossModuleOptimization.cpp
  DeadFunctionElimination.cpp
  GlobalPropertyOpt.cpp
  LetPropertiesOpts.cpp
  UsePrespecialized.cpp)
