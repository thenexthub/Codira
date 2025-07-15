# This file contains workarounds for dealing with bugs between CMake and the
# Codira compiler.

# This is a workaround to avoid crashing the Codira compiler while typechecking
# languageCore. The Codira compiler must be run num-threads=0 with WMO in all build
# configurations.
# These must be set in CMake _before_ the Codira language is enabled.
# This is manipulating undocumented CMake-internal variables that may change
# behavior at any time and should not be relied on.
# Otherwise CMake will generate the build graph incorrectly for this setup.
# Ideally this file should not need to exist.

set(CMAKE_Codira_NUM_THREADS 0)
if(POLICY CMP0157)
  set(CMAKE_Codira_COMPILE_OBJECT "<CMAKE_Codira_COMPILER> -num-threads ${CMAKE_Codira_NUM_THREADS} -c <DEFINES> <FLAGS> <INCLUDES> <SOURCE>")

  set(CMAKE_Codira_COMPILATION_MODE wholemodule)

  if(NOT CodiraStdlib_NUM_LINK_JOBS MATCHES "^[0-9]+$")
    cmake_host_system_information(RESULT CodiraStdlib_NUM_LINK_JOBS QUERY NUMBER_OF_LOGICAL_CORES)
  endif()

  set(CMAKE_Codira_LINK_EXECUTABLE "<CMAKE_Codira_COMPILER> -num-threads ${CodiraStdlib_NUM_LINK_JOBS} -emit-executable -o <TARGET> <FLAGS> <OBJECTS> <LINK_FLAGS> <LINK_LIBRARIES>")
  set(CMAKE_Codira_CREATE_SHARED_LIBRARY "<CMAKE_Codira_COMPILER> -num-threads ${CodiraStdlib_NUM_LINK_JOBS} -emit-library <CMAKE_SHARED_LIBRARY_Codira_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> ${CMAKE_Codira_IMPLIB_LINKER_FLAGS} <SONAME_FLAG> <TARGET_INSTALLNAME_DIR><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
else()
  set(CMAKE_Codira_CREATE_SHARED_LIBRARY "<CMAKE_Codira_COMPILER> -wmo -num-threads ${CMAKE_Codira_NUM_THREADS} -emit-library -o <TARGET> -module-name <LANGUAGE_MODULE_NAME> -module-link-name <LANGUAGE_LIBRARY_NAME> -emit-module -emit-module-path <LANGUAGE_MODULE> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <LANGUAGE_SOURCES> <LINK_FLAGS> <SONAME_FLAG> <TARGET_INSTALLNAME_DIR><TARGET_SONAME> ${CMAKE_Codira_IMPLIB_LINKER_FLAGS} <LINK_LIBRARIES>")

  set(CMAKE_Codira_LINK_EXECUTABLE "<CMAKE_Codira_COMPILER> -wmo -num-threads ${CMAKE_Codira_NUM_THREADS} -emit-executable -o <TARGET> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <LANGUAGE_SOURCES> <LINK_FLAGS> <LINK_LIBRARIES>")

  set(CMAKE_Codira_CREATE_STATIC_LIBRARY "<CMAKE_Codira_COMPILER> -wmo -num-threads ${CMAKE_Codira_NUM_THREADS} -emit-library -static -o <TARGET> -module-name <LANGUAGE_MODULE_NAME> -module-link-name <LANGUAGE_LIBRARY_NAME> -emit-module -emit-module-path <LANGUAGE_MODULE> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <LANGUAGE_SOURCES> <LINK_FLAGS> <LINK_LIBRARIES>")

  set(CMAKE_Codira_ARCHIVE_CREATE "<CMAKE_AR> crs <TARGET> <OBJECTS>")
  set(CMAKE_Codira_ARCHIVE_FINISH "")
endif()
