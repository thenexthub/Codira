#[=======================================================================[.rst:
FindCodiraCore
------------

Find CodiraCore, deferring to CodiraCoreConfig.cmake when requested.
This is meant to find the core library to be linked by the Supplemental libraries.

Imported Targets
^^^^^^^^^^^^^^^^

The following :prop_tgt:`IMPORTED` TARGETS may be defined:

 ``languageCore``

Hint Variables
^^^^^^^^^^^^^^

 ``SDKROOT`` (environment variable)
   Set the path to the Codira SDK Root.
   This only affects Windows builds.

 ``Codira_SDKROOT``
   Set the path to the Codira SDK installation.
   This only affects Linux and Windows builds.
   Apple builds always use the library provided by the SDK.

 ``CodiraCore_USE_STATIC_LIBS``
   Set to boolean true to find static libraries. If not
   set explicitly this is inferred from the value of
   `BUILD_SHARED_LIBS` and the current platform.

 ``CodiraCore_INCLUDE_DIR_HINTS``
   Additional paths to search the languagemodules into. These
   are prepended to the ones searched by this find module

 ``CodiraCore_LIBRARY_HINTS``
   Additional paths to search the libraries into. These
   are prepended to the ones searched by this find module

 ``CodiraShims_INCLUDE_DIR_HINTS``
   Additional paths to search the shims into. These
   are prepended to the ones searched by this find module

 ``CodiraCore_NAMES``
   Additional names for the Core library

 ``CodiraOnoneSupport_NAMES``
   Additional names for the CodiraOnoneSupport library

 ``CodiraConcurrency_NAMES``
   Additional names for the Codira_Concurrency library

Result targets
^^^^^^^^^^^^^^

If no error is generated, the following targets are available to the users

  * ``languageCore``
  * ``languageCodiraOnoneSupport``
  * ``language_Concurrency``
  * ``languageShims``

Result Variables
^^^^^^^^^^^^^^^^

The module may set the following variables if `CodiraCore_DIR` is not set.
(although we suggest relying on the targets above instead)

 ``CodiraCore_FOUND``
   true if core was found

 ``CodiraCore_INCLUDE_DIR``
   the directory containing the Codira.codemodule folder

 ``CodiraCore_LIBRARY``
   path to the languageCore library

 ``CodiraOnoneSupport_INCLUDE_DIR``
   the directory containing the CodiraOnoneSupport.codemodule folder

 ``CodiraOnoneSupport_LIBRARY``
   path to the CodiraOnoneSupport library

 ``CodiraConcurrency_INCLUDE_DIR``
   the directory containing the _Concurrency.codemodule folder

 ``CodiraConcurrency_LIBRARY``
   path to the language_Concurrency library

 ``CodiraShims_INCLUDE_DIR``
   the directory containing the Codira shims directory
   (i.e. .../usr/lib/language, not .../usr/lib/language/shims)

#]=======================================================================]

include_guard(GLOBAL)

# If the CodiraCore_DIR_FLAG is specified, look there instead. The cmake-generated
# config file is more accurate, but requires that the SDK has one available.
if(CodiraCore_DIR)
  if(CodiraCore_FIND_REQUIRED)
    list(APPEND args REQUIRED)
  endif()
  if(CodiraCore_FIND_QUIETLY)
    list(APPEND args QUIET)
  endif()
  find_package(CodiraCore NO_MODULE ${args})
  return()
endif()

include(FindPackageHandleStandardArgs)
include(PlatformInfo)

# This was loosely modelled after other find modules
# (namely FindGLEW), where the equivalent parameter
# is not stored in cache (possibly because we want
# the project importing it to be able to
# it "immediately")
if(NOT DEFINED CodiraCore_USE_STATIC_LIBS)
  set(CodiraCore_USE_STATIC_LIBS OFF)
  if(NOT BUILD_SHARED_LIBS AND NOT APPLE)
    set(CodiraCore_USE_STATIC_LIBS ON)
  endif()
endif()

if(APPLE)
  list(APPEND CodiraCore_INCLUDE_DIR_HINTS
    "${CMAKE_OSX_SYSROOT}/usr/lib/language")
  list(APPEND CodiraCore_LIBRARY_HINTS
    "${CMAKE_OSX_SYSROOT}/usr/lib/language")
  list(APPEND CodiraShims_INCLUDE_DIR_HINTS
    "${CMAKE_OSX_SYSROOT}/usr/lib/language")
  # When building for Apple platforms, CodiraCore always comes from within the
  # SDK as a tbd for a shared library in the shared cache.
  list(APPEND CodiraCore_NAMES liblanguageCore.tbd)
  list(APPEND CodiraOnoneSupport_NAMES liblanguageCodiraOnoneSupport.tbd)
  list(APPEND CodiraConcurrency_NAMES liblanguage_Concurrency.tbd)
elseif(LINUX)
  if (CodiraCore_USE_STATIC_LIBS)
    list(APPEND CodiraCore_INCLUDE_DIR_HINTS
      "${Codira_SDKROOT}/usr/lib/language_static/linux-static")
    list(APPEND CodiraCore_LIBRARY_HINTS
      "${Codira_SDKROOT}/usr/lib/language_static/linux-static")
    list(APPEND CodiraShims_INCLUDE_DIR_HINTS
      "${Codira_SDKROOT}/usr/lib/language_static"
      "${Codira_SDKROOT}/usr/lib/language")
    list(APPEND CodiraCore_NAMES liblanguageCore.a)
    list(APPEND CodiraOnoneSupport_NAMES liblanguageCodiraOnoneSupport.a)
    list(APPEND CodiraConcurrency_NAMES liblanguage_Concurrency.a)
  else()
    list(APPEND CodiraCore_INCLUDE_DIR_HINTS
      "${Codira_SDKROOT}/usr/lib/language/linux")
    list(APPEND CodiraCore_LIBRARY_HINTS
      "${Codira_SDKROOT}/usr/lib/language/linux")
    list(APPEND CodiraShims_INCLUDE_DIR_HINTS
      "${Codira_SDKROOT}/usr/lib/language")
    list(APPEND CodiraCore_NAMES liblanguageCore.so)
    list(APPEND CodiraOnoneSupport_NAMES liblanguageCodiraOnoneSupport.so)
    list(APPEND CodiraConcurrency_NAMES liblanguage_Concurrency.so)
  endif()
elseif(WIN32)
  list(APPEND CodiraCore_INCLUDE_DIR_HINTS
    "${Codira_SDKROOT}/usr/lib/language/windows"
    "$ENV{SDKROOT}/usr/lib/language/windows")
  list(APPEND CodiraCore_LIBRARY_HINTS
    "${Codira_SDKROOT}/usr/lib/language/${${PROJECT_NAME}_PLATFORM_SUBDIR}/${${PROJECT_NAME}_ARCH_SUBDIR}"
    "${Codira_SDKROOT}/usr/lib/language"
    "$ENV{SDKROOT}/usr/lib/language/${${PROJECT_NAME}_PLATFORM_SUBDIR}/${${PROJECT_NAME}_ARCH_SUBDIR}"
    "$ENV{SDKROOT}/usr/lib/language")
  list(APPEND CodiraShims_INCLUDE_DIR_HINTS
    "${Codira_SDKROOT}/usr/lib/language"
    "$ENV{SDKROOT}/usr/lib/language")
  if (CodiraCore_USE_STATIC_LIBS)
    list(APPEND CodiraCore_NAMES liblanguageCore.lib)
    list(APPEND CodiraOnoneSupport_NAMES liblanguageCodiraOnoneSupport.lib)
    list(APPEND CodiraConcurrency_NAMES liblanguage_Concurrency.lib)
  else()
    list(APPEND CodiraCore_NAMES languageCore.lib)
    list(APPEND CodiraOnoneSupport_NAMES languageCodiraOnoneSupport.lib)
    list(APPEND CodiraConcurrency_NAMES language_Concurrency.lib)
  endif()
elseif(ANDROID)
  if (CodiraCore_USE_STATIC_LIBS)
    list(APPEND CodiraCore_INCLUDE_DIR_HINTS
      "${Codira_SDKROOT}/usr/lib/language_static/android"
      "$ENV{SDKROOT}/usr/lib/language_static/android")
    list(APPEND CodiraCore_LIBRARY_HINTS
      "${Codira_SDKROOT}/usr/lib/language_static/android/${${PROJECT_NAME}_ARCH_SUBDIR}"
      "${Codira_SDKROOT}/usr/lib/language_static"
      "$ENV{SDKROOT}/usr/lib/language_static/android/${${PROJECT_NAME}_ARCH_SUBDIR}"
      "$ENV{SDKROOT}/usr/lib/language_static")
    list(APPEND CodiraShims_INCLUDE_DIR_HINTS
      "${Codira_SDKROOT}/usr/lib/language_static")
    list(APPEND CodiraCore_NAMES liblanguageCore.a)
    list(APPEND CodiraOnoneSupport_NAMES liblanguageCodiraOnoneSupport.a)
    list(APPEND CodiraConcurrency_NAMES liblanguage_Concurrency.a)
  else()
    list(APPEND CodiraCore_INCLUDE_DIR_HINTS
      "${Codira_SDKROOT}/usr/lib/language/android"
      "$ENV{SDKROOT}/usr/lib/language/android")
    list(APPEND CodiraCore_LIBRARY_HINTS
      "${Codira_SDKROOT}/usr/lib/language/android/${${PROJECT_NAME}_ARCH_SUBDIR}"
      "${Codira_SDKROOT}/usr/lib/language"
      "$ENV{SDKROOT}/usr/lib/language/android/${${PROJECT_NAME}_ARCH_SUBDIR}"
      "$ENV{SDKROOT}/usr/lib/language")
    list(APPEND CodiraShims_INCLUDE_DIR_HINTS
      "${Codira_SDKROOT}/usr/lib/language")
    list(APPEND CodiraCore_NAMES liblanguageCore.so)
    list(APPEND CodiraOnoneSupport_NAMES liblanguageCodiraOnoneSupport.so)
    list(APPEND CodiraConcurrency_NAMES liblanguage_Concurrency.so)
  endif()
else()
  message(FATAL_ERROR "FindCodiraCore.cmake module search not implemented for targeted platform\n"
  " Build Core for your platform and set `CodiraCore_DIR` to"
  " the directory containing CodiraCoreConfig.cmake\n")
endif()

find_path(CodiraCore_INCLUDE_DIR
  "Codira.codemodule"
  NO_CMAKE_FIND_ROOT_PATH
  HINTS
    ${CodiraCore_INCLUDE_DIR_HINTS})
find_library(CodiraCore_LIBRARY
  NAMES
    ${CodiraCore_NAMES}
  NO_CMAKE_FIND_ROOT_PATH
  HINTS
    ${CodiraCore_LIBRARY_HINTS})

find_path(CodiraOnoneSupport_INCLUDE_DIR
  "CodiraOnoneSupport.codemodule"
  NO_CMAKE_FIND_ROOT_PATH
  HINTS
    ${CodiraCore_INCLUDE_DIR_HINTS})
find_library(CodiraOnoneSupport_LIBRARY
  NAMES
    ${CodiraOnoneSupport_NAMES}
  NO_CMAKE_FIND_ROOT_PATH
  HINTS
    ${CodiraCore_LIBRARY_HINTS})

find_path(CodiraConcurrency_INCLUDE_DIR
  "_Concurrency.codemodule"
  NO_CMAKE_FIND_ROOT_PATH
  HINTS
    ${CodiraCore_INCLUDE_DIR_HINTS})
find_library(CodiraConcurrency_LIBRARY
  NAMES
    ${CodiraConcurrency_NAMES}
  NO_CMAKE_FIND_ROOT_PATH
  HINTS
    ${CodiraCore_LIBRARY_HINTS})

if(CodiraCore_USE_STATIC_LIBS)
  add_library(languageCore STATIC IMPORTED GLOBAL)
  add_library(languageCodiraOnoneSupport STATIC IMPORTED GLOBAL)
  add_library(language_Concurrency STATIC IMPORTED GLOBAL)
else()
  add_library(languageCore SHARED IMPORTED GLOBAL)
  add_library(languageCodiraOnoneSupport SHARED IMPORTED GLOBAL)
  add_library(language_Concurrency SHARED IMPORTED GLOBAL)
endif()

set_target_properties(languageCore PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CodiraCore_INCLUDE_DIR}")
set_target_properties(languageCodiraOnoneSupport PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CodiraOnoneSupport_INCLUDE_DIR}")
set_target_properties(language_Concurrency PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CodiraConcurrency_INCLUDE_DIR}")

if(LINUX OR ANDROID)
  set_target_properties(languageCore PROPERTIES
    IMPORTED_LOCATION "${CodiraCore_LIBRARY}")
  set_target_properties(languageCodiraOnoneSupport PROPERTIES
    IMPORTED_LOCATION "${CodiraOnoneSupport_LIBRARY}")
  set_target_properties(language_Concurrency PROPERTIES
    IMPORTED_LOCATION "${CodiraConcurrency_LIBRARY}")
else()
  set_target_properties(languageCore PROPERTIES
    IMPORTED_IMPLIB "${CodiraCore_LIBRARY}")
  set_target_properties(languageCodiraOnoneSupport PROPERTIES
    IMPORTED_IMPLIB "${CodiraOnoneSupport_LIBRARY}")
  set_target_properties(language_Concurrency PROPERTIES
    IMPORTED_IMPLIB "${CodiraConcurrency_LIBRARY}")
endif()

find_path(CodiraShims_INCLUDE_DIR "shims/module.modulemap" HINTS
  ${CodiraShims_INCLUDE_DIR_HINTS})
add_library(languageShims INTERFACE IMPORTED GLOBAL)
set_target_properties(languageShims PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CodiraShims_INCLUDE_DIR}/shims")

find_package_handle_standard_args(CodiraCore DEFAULT_MSG
  CodiraCore_LIBRARY CodiraCore_INCLUDE_DIR
  CodiraShims_INCLUDE_DIR
  CodiraOnoneSupport_LIBRARY CodiraOnoneSupport_INCLUDE_DIR
  CodiraConcurrency_LIBRARY CodiraConcurrency_INCLUDE_DIR)
