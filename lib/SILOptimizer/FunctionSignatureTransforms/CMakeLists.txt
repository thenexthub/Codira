target_sources(languageSILOptimizer PRIVATE
  FunctionSignatureOpts.cpp
  DeadArgumentTransform.cpp
  ArgumentExplosionTransform.cpp
  OwnedToGuaranteedTransform.cpp
  ExistentialSpecializer.cpp
  ExistentialTransform.cpp)
