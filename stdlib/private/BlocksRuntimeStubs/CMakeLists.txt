list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../test/cmake/modules")

include(CodiraTestUtils)

foreach(SDK ${LANGUAGE_SDKS})
  foreach(ARCH ${LANGUAGE_SDK_${SDK}_ARCHITECTURES})
    get_language_test_build_flavors(build_flavors "${SDK}")

    foreach(BUILD_FLAVOR ${build_flavors})
      get_language_test_variant_suffix(VARIANT_SUFFIX "${SDK}" "${ARCH}" "${BUILD_FLAVOR}")

      set(test_bin_dir "${LANGUAGE_BINARY_DIR}/test${VARIANT_SUFFIX}")

      add_language_target_library_single(
        BlocksRuntimeStub${VARIANT_SUFFIX}
        BlocksRuntimeStub
        SHARED NOLANGUAGERT
        ARCHITECTURE ${ARCH}
        SDK ${SDK}
        INSTALL_IN_COMPONENT never_install
        BlocksRuntime.c
      )
      set_target_properties(BlocksRuntimeStub${VARIANT_SUFFIX} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${test_bin_dir}
        LIBRARY_OUTPUT_DIRECTORY ${test_bin_dir}
        RUNTIME_OUTPUT_DIRECTORY ${test_bin_dir}
        OUTPUT_NAME BlocksRuntime)

      # When built in a unified build, ensure that we add a dependency on the
      # compiler to serialize this behind the compiler.  Otherwise, we would
      # attempt to build this before the compiler is ready, which breaks the
      # build.
      if(NOT LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER AND NOT BUILD_STANDALONE AND
          TARGET clang)
        add_dependencies(BlocksRuntimeStub${VARIANT_SUFFIX} clang)
      endif()
    endforeach()
  endforeach()
endforeach()

