add_language_unittest(CodiraSILTests
  SILBitfieldTest.cpp
)

target_link_libraries(CodiraSILTests
   PRIVATE
   languageSIL
)
