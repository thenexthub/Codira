# TODO(etcwilde) migrate this into subdir subdirectory once the migration is
# completed.
target_sources(languageCore PRIVATE
  CommandLine.cpp)
target_compile_definitions(languageCore PUBLIC
  -DLANGUAGE_STDLIB_HAS_COMMANDLINE)
target_include_directories(languageCore PRIVATE
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/include"
  "${PROJECT_BINARY_DIR}/include")
