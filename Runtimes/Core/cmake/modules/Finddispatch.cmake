#[=======================================================================[.rst:
Finddispatch
------------

Find libdispatch, deferring to dispatchConfig.cmake when requested.
This is meant to find the C implementation of libdispatch for use as the
executor for Codira concurrency. This library is not intended for use by
third-party program on Linux or Windows.

Imported Targets
^^^^^^^^^^^^^^^^

The following :prop_tgt:`IMPORTED` TARGETS may be defined:

 ``dispatch``

Hint Variables
^^^^^^^^^^^^^^

 ``language_SDKROOT``
   Set the path to the Codira SDK installation.
   This only affects Linux and Windows builds.
   Apple builds always use the library provided by the SDK.

 ``dispatch_STATIC``
   Look for the libdispatch static archive instead of the dynamic library.
   This only affects Linux. Apple builds always use the
   dynamic library provided by the SDK. Windows does not currently have a static
   libdispatch implementation.

Result Variables
^^^^^^^^^^^^^^^^

The module may set the following variables if `dispatch_DIR` is not set.

 ``dispatch_FOUND``
   true if dispatch headers and libraries were found

 ``dispatch_INCLUDE_DIR``
   the directory containing the libdispatch headers

 ``dispatch_LIBRARIES`` OR ``dispatch_IMPLIB``
   the libraries to be linked

#]=======================================================================]

# If the dispatch_DIR is specified, look there instead. The cmake-generated
# config file is more accurate, but requires that the SDK has one available.
if(dispatch_DIR)
  if(dispatch_FIND_REQUIRED)
    list(APPEND args REQUIRED)
  endif()
  if(dispatch_FIND_QUIETLY)
    list(APPEND args QUIET)
  endif()
  find_package(dispatch NO_MODULE ${args})
  return()
endif()

include(FindPackageHandleStandardArgs)

if(APPLE)
  # When building for Apple platforms, libdispatch always comes from within the
  # SDK as a tbd for a shared library in the shared cache.
  find_path(dispatch_INCLUDE_DIR "dispatch/dispatch.h")
  find_library(dispatch_IMPLIB
    NAMES "libdispatch.tbd"
    PATH "usr/lib"
    PATH_SUFFIXES system)
  add_library(dispatch SHARED IMPORTED GLOBAL)
  set_target_properties(dispatch PROPERTIES
    IMPORTED_IMPLIB "${dispatch_IMPLIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${dispatch_INCLUDE_DIR}")
  find_package_handle_standard_args(dispatch DEFAULT_MSG
    dispatch_IMPLIB dispatch_INCLUDE_DIR)
elseif(LINUX)
  if(dispatch_STATIC)
    find_path(dispatch_INCLUDE_DIR
      "dispatch/dispatch.h"
      HINTS "${Codira_SDKROOT}/usr/lib/language_static")
    find_library(dispatch_LIBRARY
      NAMES "libdispatch.a"
      HINTS "${Codira_SDKROOT}/usr/lib/language_static/linux")
    add_library(dispatch STATIC IMPORTED GLOBAL)
  else()
    find_path(dispatch_INCLUDE_DIR
      "dispatch/dispatch.h"
      HINTS "${Codira_SDKROOT}/usr/lib/language")
    find_library(dispatch_LIBRARY
      NAMES "libdispatch.so"
      HINTS "${Codira_SDKROOT}/usr/lib/language/linux")
    add_library(dispatch SHARED IMPORTED GLOBAL)
  endif()
  set_target_properties(dispatch PROPERTIES
    IMPORTED_LOCATION "${dispatch_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${dispatch_INCLUDE_DIR}")
  find_package_handle_standard_args(dispatch DEFAULT_MSG
    dispatch_LIBRARY dispatch_INCLUDE_DIR)
elseif(WIN32)
  find_path(dispatch_INCLUDE_DIR
    "dispatch/dispatch.h"
    HINTS
      "${Codira_SDKROOT}/usr/include"
      "$ENV{SDKROOT}/usr/include")
  find_library(dispatch_LIBRARY
    NAMES "dispatch.lib"
    HINTS
      "${Codira_SDKROOT}/usr/lib/language/${CodiraCore_PLATFORM_SUBDIR}/${CodiraCore_ARCH_SUBDIR}"
      "${Codira_SDKROOT}/usr/lib/language"
      "$ENV{SDKROOT}/usr/lib/language/${CodiraCore_PLATFORM_SUBDIR}/${CodiraCore_ARCH_SUBDIR}"
      "$ENV{SDKROOT}/usr/lib/language")

  add_library(dispatch SHARED IMPORTED GLOBAL)
  set_target_properties(dispatch PROPERTIES
    IMPORTED_IMPLIB "${dispatch_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${dispatch_INCLUDE_DIR}")
  find_package_handle_standard_args(dispatch DEFAULT_MSG
    dispatch_IMPLIB dispatch_INCLUDE_DIR)
else()
  message(FATAL_ERROR "Finddispatch.cmake module search not implemented for targeted platform\n"
  " Build corelibs libdispatch for your platform and set `dispatch_DIR` to"
  " the directory containing dispatchConfig.cmake\n")
endif()
