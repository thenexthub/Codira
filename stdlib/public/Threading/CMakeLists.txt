# This is the counterpart to lib/Threading/CMakeLists.txt.  Any updates
# need to be reflected in both places.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules")
include(AddCodiraStdlib)

# This should *not* include ThreadSanitizer.cpp, as that is part of liblanguageCore
add_language_target_library(languageThreading OBJECT_LIBRARY
  "${LANGUAGE_SOURCE_DIR}/lib/Threading/C11.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Threading/Linux.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Threading/Pthreads.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Threading/Win32.cpp"
  INSTALL_IN_COMPONENT never_install)
