# Toolchain-only build products

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake/modules)
include(StdlibOptions)
include(AddCodiraStdlib)
include(CompatibilityLibs)

set(CXX_COMPILE_FLAGS)
set(CXX_LINK_FLAGS)


# Build the runtime with -Wall to catch, e.g., uninitialized variables
# warnings.
if(LANGUAGE_COMPILER_IS_MSVC_LIKE)
  list(APPEND compile_flags "/W3")
else()
  list(APPEND compile_flags "-Wall")
endif()

list(APPEND compile_flags
  # C++ code in the runtime and standard library should generally avoid
  # introducing static constructors or destructors.
  "-Wglobal-constructors"
  "-Wexit-time-destructors")

foreach(flag ${compile_flags})
  check_cxx_compiler_flag("${flag}" cxx_compiler_flag_${flag}_is_supported)
  if(cxx_compiler_flag_${flag}_is_supported)
    list(APPEND CXX_COMPILE_FLAGS "${flag}")
  endif()
endforeach()
unset(compile_flags)


if("Thread" IN_LIST LANGUAGE_RUNTIME_USE_SANITIZERS)
  list(APPEND CXX_LINK_FLAGS "-fsanitize=thread")
endif()

# Compatibility libraries build in a special alternate universe that can't
# directly link to most OS runtime libraries, and have to access the
# runtime being patched only through public ABI.
list(APPEND CXX_COMPILE_FLAGS "-DLANGUAGE_COMPATIBILITY_LIBRARY=1")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

if(LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT)
  add_subdirectory(legacy_layouts)
  add_subdirectory(Compatibility50)
  add_subdirectory(Compatibility51)
  add_subdirectory(Compatibility56)
  add_subdirectory(CompatibilitySpan)
  add_subdirectory(CompatibilityDynamicReplacements)
  add_subdirectory(CompatibilityConcurrency)
  add_subdirectory(CompatibilityThreading)
  add_subdirectory(CompatibilityPacks)

  # This is a convenience target to have the list
  # of all the compatibility libraries needed to build
  # host tools in a single place
  add_library(HostCompatibilityLibs INTERFACE)
  get_compatibility_libs(
    "${LANGUAGE_HOST_VARIANT_SDK}"
    "${LANGUAGE_HOST_VARIANT_ARCH}"
    compatibility_libs
    )
  target_link_libraries(HostCompatibilityLibs INTERFACE
    ${compatibility_libs}
    )
  set_property(GLOBAL APPEND PROPERTY LANGUAGE_BUILDTREE_EXPORTS HostCompatibilityLibs)
endif()
