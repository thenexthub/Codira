add_language_unittest(CodiraRemangleTests
  RemangleTest.cpp
  )

target_link_libraries(CodiraRemangleTests
  PRIVATE
  languageBasic
  languageDemangling
  LLVMTestingSupport
  )
