add_language_unittest(CodiraSILOptimizerTests
  PartitionUtilsTest.cpp
)

target_link_libraries(CodiraSILOptimizerTests
   PRIVATE
   languageSILOptimizer
   languageIRGen
   languageAST
   languageFrontend
)
