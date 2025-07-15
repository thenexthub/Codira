#===--- CMakeLists.txt - Macro support libraries ------------------------===#
#
# Copyright (c) NeXTHub Corporation. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# Author(-s): Tunjay Akbarli

#
#===----------------------------------------------------------------------===#

# The location for host plugins, i.e., macros.
set(LANGUAGE_HOST_PLUGINS_DEST_DIR ${LANGUAGE_HOST_LIBRARIES_DEST_DIR}/plugins)

# Add a new Codira macro library, which is built for the host and installed
# into lib/language/host/plugins.
#
# Usage:
#   add_language_macro_library(name
#     source1 [source2 source3 ...])
#
# name
#   Name of the library (e.g., CodiraMacros).
#
# source1 ...
#   Sources to add into this library.
function(add_language_macro_library name)
  set(ASML_SOURCES ${ARGN})

  # Add the library.
  add_pure_language_host_library(${name} SHARED
    PACKAGE_NAME Toolchain
    ${ASML_SOURCES})

  # If we don't have the Codira language parser, bail out, because the above
  # add_pure_language_host_library did nothing.
  if (NOT LANGUAGE_BUILD_LANGUAGE_SYNTAX)
    return()
  endif()

  # Add rpath to 'lib/{platform}'
  file(RELATIVE_PATH relpath_to_lib
    "${LANGUAGE_HOST_PLUGINS_DEST_DIR}"
    "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
  )
  _set_pure_language_link_flags(${name} "${relpath_to_lib}")

  # Add rpath to 'lib/host'
  if(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
    set_property(TARGET ${name}
      APPEND PROPERTY INSTALL_RPATH "$ORIGIN/..")
  endif()

  # Install into the plugin directory.
  set_target_properties(${name}
    PROPERTIES
      ARCHIVE_OUTPUT_DIRECTORY "${LANGUAGE_HOST_PLUGINS_DEST_DIR}"
      LIBRARY_OUTPUT_DIRECTORY "${LANGUAGE_HOST_PLUGINS_DEST_DIR}"
  )

  set(destination_dir "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host/plugins")

  language_install_in_component(TARGETS ${name}
    RUNTIME
      DESTINATION bin
      COMPONENT compiler
    LIBRARY
      DESTINATION "${destination_dir}"
      COMPONENT compiler
    ARCHIVE
      DESTINATION "${destination_dir}"
      COMPONENT compiler)

  # Export this macro plugin target.
  set_property(GLOBAL APPEND PROPERTY LANGUAGE_MACRO_PLUGINS ${name})
endfunction()

add_subdirectory(Sources/CodiraMacros)
add_subdirectory(Sources/ObservationMacros)
