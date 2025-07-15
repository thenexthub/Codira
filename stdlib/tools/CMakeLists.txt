# Keep in sync with test/CMakeLists.txt: language-reflection-test is
# only used when testing dynamic stdlib.
if(LANGUAGE_BUILD_REMOTE_MIRROR AND
  LANGUAGE_BUILD_DYNAMIC_STDLIB AND
  (LANGUAGE_INCLUDE_TESTS OR LANGUAGE_INCLUDE_TEST_BINARIES))
  add_subdirectory(language-reflection-test)
endif()
