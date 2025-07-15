list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(CodiraTestUtils)

function(language_configure_lit_site_cfg source_path destination_path installed_name)
  if (CMAKE_CFG_INTDIR STREQUAL ".")
    set(LANGUAGE_BUILD_MODE ".")
  else ()
    set(LANGUAGE_BUILD_MODE "%(build_mode)s")
  endif ()

  string(REPLACE ${CMAKE_CFG_INTDIR} ${LANGUAGE_BUILD_MODE} TOOLCHAIN_TOOLS_DIR ${TOOLCHAIN_TOOLS_BINARY_DIR})
  string(REPLACE ${CMAKE_CFG_INTDIR} ${LANGUAGE_BUILD_MODE} TOOLCHAIN_LIBS_DIR  ${TOOLCHAIN_LIBRARY_DIR})

  set(LIT_LANGUAGELIB_DIR ${LANGUAGELIB_DIR})

  configure_file("${source_path}" "${destination_path}" @ONLY)

  if(NOT "${installed_name}" STREQUAL "")
    language_install_in_component(FILES "${destination_path}"
                               RENAME "${installed_name}"
                               DESTINATION "share/language/testsuite"
                               COMPONENT testsuite-tools)
  endif()
endfunction()

function(language_generate_lit_language_features_cfg output_path)
  if(LANGUAGE_COMPILER_IS_MSVC_LIKE)
    set(test_lit_language_features_cmd
      "${CMAKE_C_COMPILER}" "/P" "/EP" "/I" "${LANGUAGE_MAIN_INCLUDE_DIR}" "/TC"
      "/Fi<<<OUTPUT_FILE>>>")
  else()
    set(test_lit_language_features_cmd
      "${CMAKE_C_COMPILER}" "-E" "-P" "-I${LANGUAGE_MAIN_INCLUDE_DIR}" "-x" "c" "-o"
      "<<<OUTPUT_FILE>>>")
  endif()

  list(
    TRANSFORM test_lit_language_features_cmd
    REPLACE "<<<OUTPUT_FILE>>>" "${output_path}")
  add_custom_command(
    OUTPUT "${output_path}"
    COMMAND
      ${test_lit_language_features_cmd}
      "${CMAKE_CURRENT_SOURCE_DIR}/lit.code-features.cfg.inc"
    DEPENDS
      "lit.code-features.cfg.inc"
      "${LANGUAGE_MAIN_INCLUDE_DIR}/language/Basic/Features.def"
  )

  # Execute during generation once, so one can run Lit without rebuilding the
  # test suite dependencies first.
  execute_process(
    COMMAND
      ${test_lit_language_features_cmd}
      "${CMAKE_CURRENT_SOURCE_DIR}/lit.code-features.cfg.inc"
  )
endfunction()

function(normalize_boolean_spelling var_name)
  if(${var_name})
    set("${var_name}" TRUE PARENT_SCOPE)
  else()
    set("${var_name}" FALSE PARENT_SCOPE)
  endif()
endfunction()

function(get_test_dependencies SDK result_var_name)
  set(deps)

  if(LANGUAGE_BUILD_STDLIB)
    list(APPEND deps CodiraUnitTests)
  endif()

  if(LANGUAGE_BUILD_SDK_OVERLAY OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
    list(APPEND deps sdk-overlay)
  endif()

  set(deps_binaries)

  if (LANGUAGE_BUILD_LIBEXEC AND LANGUAGE_ENABLE_BACKTRACING)
    # Only add if `language-backtrace` builds for this SDK
    if (TARGET "language-backtrace-${SDK}")
      list(APPEND deps_binaries
        "language-backtrace-${SDK}")
    endif()
  endif()

  if (LANGUAGE_INCLUDE_TOOLS)
    list(APPEND deps_binaries
      libMockPlugin
      lldb-moduleimport-test
      language-frontend
      language-demangle
      language-demangle-yamldump
      language-ide-test
      language-refactor
      language-reflection-dump
      language-remoteast-test
      language-scan-test)

    if(LANGUAGE_BUILD_SOURCEKIT)
      list(APPEND deps_binaries sourcekitd-test complete-test)
    endif()

    if(LANGUAGE_BUILD_LANGUAGE_SYNTAX)
      list(APPEND deps_binaries language-plugin-server)
    endif()
  endif()

  if(NOT LANGUAGE_BUILT_STANDALONE)
    list(APPEND deps_binaries
      c-index-test
      CASPluginTest
      clang
      clang-cas-test
      count
      dsymutil
      FileCheck
      llc
      toolchain-ar
      toolchain-as
      toolchain-bcanalyzer
      toolchain-cas
      toolchain-cov
      toolchain-dis
      toolchain-dwarfdump
      toolchain-link
      toolchain-nm
      toolchain-objdump
      toolchain-profdata
      toolchain-readelf
      toolchain-readobj
      toolchain-size
      toolchain-strings
      toolchain-readtapi
      not
      split-file)
  endif()

  if(("${SDK}" STREQUAL "IOS") OR
     ("${SDK}" STREQUAL "TVOS") OR
     ("${SDK}" STREQUAL "WATCHOS") OR
     ("${SDK}" STREQUAL "XROS") OR
     ("${SDK}" STREQUAL "OSX") OR
     ("${SDK}" STREQUAL "IOS_SIMULATOR") OR
     ("${SDK}" STREQUAL "TVOS_SIMULATOR") OR
     ("${SDK}" STREQUAL "WATCHOS_SIMULATOR") OR
     ("${SDK}" STREQUAL "XROS_SIMULATOR") OR
     ("${SDK}" STREQUAL "FREESTANDING") OR
     ("${SDK}" STREQUAL "LINUX") OR
     ("${SDK}" STREQUAL "LINUX_STATIC") OR
     ("${SDK}" STREQUAL "CYGWIN") OR
     ("${SDK}" STREQUAL "FREEBSD") OR
     ("${SDK}" STREQUAL "OPENBSD") OR
     ("${SDK}" STREQUAL "ANDROID") OR
     ("${SDK}" STREQUAL "WINDOWS") OR
     ("${SDK}" STREQUAL "HAIKU") OR
     ("${SDK}" STREQUAL "WASI"))
    # No extra dependencies.
  else()
    message(FATAL_ERROR "Unknown SDK: ${SDK}")
  endif()

  # Just use target names for dependency generation. This works for both Xcode
  # and non-Xcode build systems. In the case of Xcode, its build paths have a
  # configuration variable in them, so CMake can't match them at compile time.
  list(APPEND deps ${deps_binaries})

  set("${result_var_name}" "${deps}" PARENT_SCOPE)
endfunction()

set(LIT "${TOOLCHAIN_MAIN_SRC_DIR}/utils/lit/lit.py")

set(LANGUAGE_LIT_ARGS "" CACHE STRING "Arguments to pass to lit")

set(LANGUAGE_LIT_ENVIRONMENT "" CACHE STRING "Environment to use for lit invocations")

option(LANGUAGE_TEST_USE_LEAKS "Run Codira stdlib tests under leaks" FALSE)

function(setup_lit_args ARGS_VAR_OUT tested_sdk test_results_dir resource_dir_override)
  set(language_lit_args_result)

  if(NOT LANGUAGE_INCLUDE_TOOLS)
    if(LANGUAGE_RUN_TESTS_WITH_HOST_COMPILER)
      precondition(CMAKE_Codira_COMPILER MESSAGE "Can only run tests if a Codira compiler is specified")
      get_filename_component(LANGUAGE_COMPILER_DIR "${CMAKE_Codira_COMPILER}" DIRECTORY)
      precondition(LANGUAGE_COMPILER_DIR)
      # We assume that we are building against a toolchain where all tools are
      # next to languagec.
      list(APPEND language_lit_args_result
        "--path=${LANGUAGE_COMPILER_DIR}")
    else()
      list(APPEND language_lit_args_result
        "--path=${LANGUAGE_NATIVE_TOOLCHAIN_TOOLS_PATH}"
        "--param" "language_native_toolchain_tools_path=${LANGUAGE_NATIVE_TOOLCHAIN_TOOLS_PATH}"
        "--path=${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}"
        "--param" "language_native_clang_tools_path=${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}"
        "--path=${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}"
        "--param" "language_native_language_tools_path=${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}"
        )
    endif()
    if(LANGUAGE_BUILD_STDLIB)
      if(resource_dir_override)
        list(APPEND language_lit_args_result
             "--param" "test_resource_dir=${resource_dir_override}")
      # If building only static stdlib, use `language_static` resources directory.
      elseif(NOT LANGUAGE_BUILD_DYNAMIC_STDLIB AND LANGUAGE_BUILD_STATIC_STDLIB)
        list(APPEND language_lit_args_result
             "--param" "test_resource_dir=${LANGUAGESTATICLIB_DIR}")
      else()
        list(APPEND language_lit_args_result
             "--param" "test_resource_dir=${LANGUAGELIB_DIR}")
      endif()
    endif()
  endif()

  if (LANGUAGE_TEST_USE_LEAKS)
    list(APPEND language_lit_args_result "--param" "leaks-all")
  endif()

  if (LANGUAGE_ENABLE_ARRAY_COW_CHECKS)
    list(APPEND language_lit_args_result
         "--param" "array_cow_checks")
  endif()

  if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
    list(APPEND language_lit_args_result
         "--param" "build_mode=${CMAKE_CFG_INTDIR}")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING)
    list(APPEND language_lit_args_result "--param" "differentiable_programming")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY)
    list(APPEND language_lit_args_result "--param" "concurrency")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED)
    list(APPEND language_lit_args_result "--param" "distributed")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING)
    list(APPEND language_lit_args_result "--param" "string_processing")
  endif()

  if(LANGUAGE_ENABLE_BACKTRACING)
    list(APPEND language_lit_args_result "--param" "backtracing")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION)
    list(APPEND language_lit_args_result "--param" "observation")
  endif()

  if(LANGUAGE_ENABLE_SYNCHRONIZATION)
    list(APPEND language_lit_args_result "--param" "synchronization")
  endif()

  if(LANGUAGE_ENABLE_VOLATILE)
    list(APPEND language_lit_args_result "--param" "volatile")
  endif()

  if(LANGUAGE_ENABLE_RUNTIME_MODULE)
    list(APPEND language_lit_args_result "--param" "runtime_module")
  endif()

  if(LANGUAGE_BUILD_REMOTE_MIRROR)
    list(APPEND language_lit_args_result "--param" "remote_mirror")
  endif()

  list(APPEND language_lit_args_result "--param" "threading=${LANGUAGE_SDK_${tested_sdk}_THREADING_PACKAGE}")

  # Enable on-crash backtracing if supported
  if(("${tested_sdk}" STREQUAL "OSX" OR "${tested_sdk}" STREQUAL "LINUX")
      AND NOT LANGUAGE_ASAN_BUILD)
    list(APPEND language_lit_args_result "--param" "backtrace_on_crash")
  endif()

  execute_process(COMMAND
      $<TARGET_FILE:Python3::Interpreter> "-c" "import psutil"
      RESULT_VARIABLE python_psutil_status
      TIMEOUT 1 # second
      ERROR_QUIET)
  if(NOT python_psutil_status)
    list(APPEND language_lit_args_result "--timeout=3000") # 50 minutes
  endif()

  list(APPEND language_lit_args_result "--xunit-xml-output=${test_results_dir}/lit-tests.xml")

  if(NOT LANGUAGE_BUILD_STDLIB AND NOT LANGUAGE_PATH_TO_EXTERNAL_STDLIB_BUILD)
    list(APPEND language_lit_args_result
        "--param" "test_sdk_overlay_dir=${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${tested_sdk}_LIB_SUBDIR}")
  endif()


  set(language_lit_args_result "${language_lit_args_result} ${LANGUAGE_LIT_ARGS} ${TOOLCHAIN_LIT_ARGS}")
  separate_arguments(language_lit_args_result)

  set(${ARGS_VAR_OUT} ${language_lit_args_result} PARENT_SCOPE)
endfunction()

if (TOOLCHAIN_USE_SANITIZER STREQUAL "Address")
  set(LANGUAGE_ASAN_BUILD TRUE)
endif()

# Normalize spelling of boolean values.
normalize_boolean_spelling(TOOLCHAIN_ENABLE_ASSERTIONS)
normalize_boolean_spelling(LANGUAGE_STDLIB_ASSERTIONS)
normalize_boolean_spelling(LANGUAGE_AST_VERIFIER)
normalize_boolean_spelling(LANGUAGE_ASAN_BUILD)
normalize_boolean_spelling(LANGUAGE_ENABLE_SOURCEKIT_TESTS)
normalize_boolean_spelling(LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING)
normalize_boolean_spelling(LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY)
normalize_boolean_spelling(LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED)
normalize_boolean_spelling(LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING)
normalize_boolean_spelling(LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION)
normalize_boolean_spelling(LANGUAGE_ENABLE_EXPERIMENTAL_POINTER_BOUNDS)
normalize_boolean_spelling(LANGUAGE_ENABLE_MACCATALYST)
normalize_boolean_spelling(LANGUAGE_RUN_TESTS_WITH_HOST_COMPILER)
normalize_boolean_spelling(LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER)
normalize_boolean_spelling(LANGUAGE_OPTIMIZED)
normalize_boolean_spelling(LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY)
normalize_boolean_spelling(LANGUAGE_ENABLE_REFLECTION)
normalize_boolean_spelling(LANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS)
normalize_boolean_spelling(LANGUAGE_STDLIB_ENABLE_DEBUG_PRECONDITIONS_IN_RELEASE)
normalize_boolean_spelling(LANGUAGE_HAVE_LIBXML2)
normalize_boolean_spelling(LANGUAGE_INCLUDE_TOOLS)
normalize_boolean_spelling(LANGUAGE_STDLIB_STATIC_PRINT)
normalize_boolean_spelling(LANGUAGE_STDLIB_ENABLE_UNICODE_DATA)
normalize_boolean_spelling(LANGUAGE_ENABLE_DISPATCH)
normalize_boolean_spelling(LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP)
normalize_boolean_spelling(LANGUAGE_ENABLE_BACKTRACING)
normalize_boolean_spelling(LANGUAGE_ENABLE_RUNTIME_MODULE)
normalize_boolean_spelling(LANGUAGE_BUILD_LANGUAGE_SYNTAX)
normalize_boolean_spelling(LANGUAGE_ENABLE_SYNCHRONIZATION)
normalize_boolean_spelling(LANGUAGE_ENABLE_VOLATILE)
normalize_boolean_spelling(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
normalize_boolean_spelling(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB_CROSS_COMPILING)
normalize_boolean_spelling(LANGUAGE_BUILD_REMOTE_MIRROR)
is_build_type_optimized("${LANGUAGE_STDLIB_BUILD_TYPE}" LANGUAGE_OPTIMIZED)

# Get 'LANGUAGE_HOST_SDKROOT' for lit.site.cfg.in
set(LANGUAGE_HOST_SDKROOT "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_ARCH_${LANGUAGE_HOST_VARIANT_ARCH}_PATH}")

set(profdata_merge_worker
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/profdata_merge/main.py")

set(TEST_MODES
    optimize_none optimize optimize_unchecked optimize_size
    optimize_none_with_implicit_dynamic
    optimize_with_implicit_dynamic
    only_executable only_non_executable
    with_cxx_interop
)
set(TEST_SUBSETS
    primary
    validation
    all
    only_validation
    only_long
    only_stress
)

if(NOT "${COVERAGE_DB}" STREQUAL "")
  add_custom_target("touch-covering-tests"
      COMMAND "${LANGUAGE_SOURCE_DIR}/utils/coverage/coverage-touch-tests" "--language-dir" "${LANGUAGE_SOURCE_DIR}" "--coverage-db" "${COVERAGE_DB}"
      COMMENT "Touching covering tests")
endif()

foreach(SDK ${LANGUAGE_SDKS})
  foreach(ARCH ${LANGUAGE_SDK_${SDK}_ARCHITECTURES})
    # macCatalyst needs to run two sets of tests: one with the normal macosx target triple
    # and one with the macCatalyst ios-macabi triple.  The build_flavors list will
    # have only the "default" flavor for all SDKs and architectures except
    # OSX when macCatalyst support is enabled.
    get_language_test_build_flavors(build_flavors "${SDK}")

    foreach(BUILD_FLAVOR ${build_flavors})
      # Configure variables for this subdirectory.
      set(VARIANT_SDK "${LANGUAGE_SDK_${SDK}_ARCH_${ARCH}_PATH}")
      get_language_test_variant_suffix(VARIANT_SUFFIX "${SDK}" "${ARCH}" "${BUILD_FLAVOR}")
      get_language_test_variant_suffix(DEFAULT_OSX_VARIANT_SUFFIX "${SDK}" "${ARCH}" "default")
      get_language_test_versioned_target_triple(VARIANT_TRIPLE "${SDK}" "${ARCH}" "${LANGUAGE_SDK_${SDK}_DEPLOYMENT_VERSION}" "${BUILD_FLAVOR}")

      # A directory where to put the xUnit-style XML test results.
      set(LANGUAGE_TEST_RESULTS_DIR
          "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/language-test-results/${VARIANT_TRIPLE}")

      set(command_clean_test_results_dir
          COMMAND "${CMAKE_COMMAND}" -E remove_directory "${LANGUAGE_TEST_RESULTS_DIR}"
          COMMAND "${CMAKE_COMMAND}" -E make_directory "${LANGUAGE_TEST_RESULTS_DIR}")

      set(test_bin_dir "${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}")
      set(validation_test_bin_dir
          "${CMAKE_CURRENT_BINARY_DIR}/../validation-test${VARIANT_SUFFIX}")

      if(TOOLCHAIN_ENABLE_LIBXML2)
        set(LANGUAGE_HAVE_LIBXML2 TRUE)
      else()
        set(LANGUAGE_HAVE_LIBXML2 FALSE)
      endif()

      set(VARIANT_EXTERNAL_EMBEDDED_PLATFORM FALSE)
      set(VARIANT_EXTERNAL_EMBEDDED_DEVICE)

      language_configure_lit_site_cfg(
          "${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in"
          "${test_bin_dir}/lit.site.cfg"
          "test${VARIANT_SUFFIX}.lit.site.cfg")

      language_configure_lit_site_cfg(
          "${CMAKE_CURRENT_SOURCE_DIR}/Unit/lit.site.cfg.in"
          "${test_bin_dir}/Unit/lit.site.cfg"
          "")

      language_configure_lit_site_cfg(
          "${CMAKE_CURRENT_SOURCE_DIR}/../validation-test/lit.site.cfg.in"
          "${validation_test_bin_dir}/lit.site.cfg"
          "validation-test${VARIANT_SUFFIX}.lit.site.cfg")

      language_generate_lit_language_features_cfg("${test_bin_dir}/lit.code-features.cfg")
      language_generate_lit_language_features_cfg("${validation_test_bin_dir}/lit.code-features.cfg")
      add_custom_target(lit_language_features_cfg_${VARIANT_SUFFIX}
        DEPENDS
          "${test_bin_dir}/lit.code-features.cfg"
          "${validation_test_bin_dir}/lit.code-features.cfg")

      set(test_dependencies)
      get_test_dependencies("${SDK}" test_dependencies)

      # Keep in sync with stdlib/tools/CMakeLists.txt: language-reflection-test is
      # only used when testing dynamic stdlib.
      if(LANGUAGE_BUILD_DYNAMIC_STDLIB AND LANGUAGE_INCLUDE_TESTS)
        list(APPEND test_dependencies BlocksRuntimeStub${VARIANT_SUFFIX})

        list(APPEND test_dependencies
            "language-test-stdlib-${LANGUAGE_SDK_${SDK}_LIB_SUBDIR}")

        if(BUILD_FLAVOR STREQUAL "ios-like")
          # When testing the iOS-like build flavor, use the normal macOS
          # language-reflection-test-tool. That tool runs out of process so it
          # doesn't need to be build for macCatalyst.
          list(APPEND test_dependencies
              "language-reflection-test${DEFAULT_OSX_VARIANT_SUFFIX}")
        else()
          list(APPEND test_dependencies
              "language-reflection-test${VARIANT_SUFFIX}")
        endif()
      endif()

      if(TARGET "embedded-libraries")
        list(APPEND test_dependencies "embedded-libraries")
      endif()

      if(NOT "${COVERAGE_DB}" STREQUAL "")
        list(APPEND test_dependencies "touch-covering-tests")
      endif()

      list(APPEND test_dependencies
        "languageStdlibCollectionUnittest-${LANGUAGE_SDK_${SDK}_LIB_SUBDIR}"
        "languageStdlibUnicodeUnittest-${LANGUAGE_SDK_${SDK}_LIB_SUBDIR}")

      set(validation_test_dependencies)

      list(APPEND test_dependencies lit_language_features_cfg_${VARIANT_SUFFIX})
      list(APPEND validation_test_dependencies lit_language_features_cfg_${VARIANT_SUFFIX})

      set(command_upload_stdlib)
      set(command_upload_language_reflection_test)
      if("${SDK}" STREQUAL "IOS" OR "${SDK}" STREQUAL "TVOS" OR "${SDK}" STREQUAL "WATCHOS" OR "${SDK}" STREQUAL "XROS")
        # These are supported testing SDKs, but their implementation of
        # `command_upload_stdlib` is hidden.
      elseif("${SDK}" STREQUAL "ANDROID" AND NOT "${LANGUAGE_HOST_VARIANT}" STREQUAL "android")
        # This adb setup is only needed when cross-compiling for Android, so the
        # second check above makes sure we don't bother when the host is Android.
        if("${LANGUAGE_ANDROID_DEPLOY_DEVICE_PATH}" STREQUAL "")
          message(FATAL_ERROR
              "When running Android host tests, you must specify the directory on the device "
              "to which Codira build products will be deployed.")
        endif()

        # Warning: This step will fail if you do not have an Android device
        #          connected via USB. See docs/Android.md for details on
        #          how to run the test suite for Android.
        set(command_upload_stdlib
            COMMAND
                # Reboot the device and remove everything in its tmp
                # directory. Build products and test executables are pushed
                # to that directory when running the test suite.
                $<TARGET_FILE:Python3::Interpreter> "${LANGUAGE_SOURCE_DIR}/utils/android/adb_clean.py"
            COMMAND
                $<TARGET_FILE:Python3::Interpreter> "${LANGUAGE_SOURCE_DIR}/utils/android/adb_push_built_products.py"
                --ndk "${LANGUAGE_ANDROID_NDK_PATH}"
                --destination "${LANGUAGE_ANDROID_DEPLOY_DEVICE_PATH}"
                --destination-arch "${ARCH}"
                # Build products like liblanguageCore.so.
                "${LANGUAGELIB_DIR}/android")
      endif()
      add_custom_target("upload-stdlib${VARIANT_SUFFIX}"
          ${command_upload_stdlib}
          ${command_upload_language_reflection_test}
          COMMENT "Uploading stdlib")

      foreach(test_mode ${TEST_MODES})
        foreach(test_subset ${TEST_SUBSETS})
          set(directories)
          set(dependencies ${test_dependencies})

          if((test_subset STREQUAL "primary") OR
             (test_subset STREQUAL "validation") OR
             (test_subset STREQUAL "only_long") OR
             (test_subset STREQUAL "only_stress") OR
             (test_subset STREQUAL "all"))
            list(APPEND directories "${test_bin_dir}")
          endif()
          if((test_subset STREQUAL "validation") OR
             (test_subset STREQUAL "only_validation") OR
             (test_subset STREQUAL "only_long") OR
             (test_subset STREQUAL "only_stress") OR
             (test_subset STREQUAL "all"))
            list(APPEND directories "${validation_test_bin_dir}")
            list(APPEND dependencies ${validation_test_dependencies})
          endif()

          if("${LANGUAGE_SDK_${SDK}_OBJECT_FORMAT}" STREQUAL "ELF")
            list(APPEND dependencies languageImageRegistration${VARIANT_SUFFIX})
          endif()

          set(test_subset_target_suffix "-${test_subset}")
          if(test_subset STREQUAL "primary")
            set(test_subset_target_suffix "")
          endif()

          set(test_mode_target_suffix "")
          if(NOT test_mode STREQUAL "optimize_none")
            set(test_mode_target_suffix "-${test_mode}")
          endif()

          set(maybe_command_upload_stdlib)
          if(NOT test_mode STREQUAL "only_non_executable")
            set(maybe_command_upload_stdlib ${command_upload_stdlib})
          endif()

          setup_lit_args(final_lit_args "${SDK}" "${LANGUAGE_TEST_RESULTS_DIR}" OFF)

          set(test_target_name
              "check-language${test_subset_target_suffix}${test_mode_target_suffix}${VARIANT_SUFFIX}")
          add_custom_target("${test_target_name}"
              ${maybe_command_upload_stdlib}
              ${command_upload_language_reflection_test}
              ${command_clean_test_results_dir}
              COMMAND
                ${CMAKE_COMMAND} -E env ${LANGUAGE_LIT_ENVIRONMENT}
                $<TARGET_FILE:Python3::Interpreter> "${LIT}"
                ${final_lit_args}
                "--param" "language_test_subset=${test_subset}"
                "--param" "language_test_mode=${test_mode}"
                ${directories}
              DEPENDS ${dependencies}
              COMMENT "Running ${test_subset} Codira tests for ${VARIANT_TRIPLE}"
              USES_TERMINAL)

          set(test_dependencies_target_name
              "language${test_subset_target_suffix}${test_mode_target_suffix}${VARIANT_SUFFIX}-test-depends")
          add_custom_target("${test_dependencies_target_name}"
              DEPENDS ${dependencies})

          add_custom_target("${test_target_name}-custom"
              ${command_upload_stdlib}
              ${command_upload_language_reflection_test}
              ${command_clean_test_results_dir}
              COMMAND
                ${CMAKE_COMMAND} -E env ${LANGUAGE_LIT_ENVIRONMENT}
                $<TARGET_FILE:Python3::Interpreter> "${LIT}"
                ${final_lit_args}
                "--param" "language_test_subset=${test_subset}"
                "--param" "language_test_mode=${test_mode}"
                ${LANGUAGE_LIT_TEST_PATHS}
              DEPENDS ${dependencies}
              COMMENT "Running ${test_subset} Codira tests for ${VARIANT_TRIPLE} from custom test locations"
              USES_TERMINAL)
          set_property(TARGET
              "${test_target_name}"
              "${test_target_name}-custom"
              "${test_dependencies_target_name}"
              PROPERTY FOLDER "Tests/check-language")
        endforeach()
      endforeach()
    endforeach()
  endforeach()
endforeach()

if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB_CROSS_COMPILING)
  set(VARIANT_SUFFIX "-embedded-armv7")
  set(VARIANT_TRIPLE "armv7em-none-none-eabi")
  set(VARIANT_EXTERNAL_EMBEDDED_PLATFORM TRUE)
  set(VARIANT_EXTERNAL_EMBEDDED_DEVICE "arm-qemu-stm32f4")
  set(LANGUAGE_TEST_RESULTS_DIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/language-test-results/${VARIANT_TRIPLE}")
  language_configure_lit_site_cfg(
      "${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in"
      "${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}/lit.site.cfg"
      "test${VARIANT_SUFFIX}.lit.site.cfg")
  language_generate_lit_language_features_cfg("${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}/lit.code-features.cfg")

  set(VARIANT_SUFFIX "-embedded-avr")
  set(VARIANT_TRIPLE "avr-none-none-elf")
  set(VARIANT_EXTERNAL_EMBEDDED_PLATFORM TRUE)
  set(VARIANT_EXTERNAL_EMBEDDED_DEVICE "avr-qemu-atmega2560")
  set(LANGUAGE_TEST_RESULTS_DIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/language-test-results/${VARIANT_TRIPLE}")
  language_configure_lit_site_cfg(
      "${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in"
      "${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}/lit.site.cfg"
      "test${VARIANT_SUFFIX}.lit.site.cfg")
  language_generate_lit_language_features_cfg("${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}/lit.code-features.cfg")

  set(VARIANT_SUFFIX "-embedded-riscv32")
  set(VARIANT_TRIPLE "riscv32-none-none-eabi")
  set(VARIANT_EXTERNAL_EMBEDDED_PLATFORM TRUE)
  set(VARIANT_EXTERNAL_EMBEDDED_DEVICE "riscv32-qemu-virt")
  set(LANGUAGE_TEST_RESULTS_DIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/language-test-results/${VARIANT_TRIPLE}")
  language_configure_lit_site_cfg(
      "${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in"
      "${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}/lit.site.cfg"
      "test${VARIANT_SUFFIX}.lit.site.cfg")
  language_generate_lit_language_features_cfg("${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}/lit.code-features.cfg")

  set(VARIANT_SUFFIX "-embedded-wasi")
  set(VARIANT_TRIPLE "wasm32-unknown-wasip1")
  set(VARIANT_EXTERNAL_EMBEDDED_PLATFORM FALSE)
  set(VARIANT_EXTERNAL_EMBEDDED_DEVICE)
  set(LANGUAGE_TEST_RESULTS_DIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/language-test-results/${VARIANT_TRIPLE}")
  language_configure_lit_site_cfg(
      "${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in"
      "${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}/lit.site.cfg"
      "test${VARIANT_SUFFIX}.lit.site.cfg")
  language_generate_lit_language_features_cfg("${CMAKE_CURRENT_BINARY_DIR}${VARIANT_SUFFIX}/lit.code-features.cfg")
endif()

# Add shortcuts for the default variant.
foreach(test_mode ${TEST_MODES})
  foreach(test_subset ${TEST_SUBSETS})
    set(test_mode_target_suffix "")
    if(NOT test_mode STREQUAL "optimize_none")
      set(test_mode_target_suffix "-${test_mode}")
    endif()
    set(test_subset_target_suffix "-${test_subset}")
    if(test_subset STREQUAL "primary")
      set(test_subset_target_suffix "")
    endif()

    set(test_target_name
        "check-language${test_subset_target_suffix}${test_mode_target_suffix}")
    add_custom_target("${test_target_name}"
        DEPENDS "${test_target_name}${LANGUAGE_PRIMARY_VARIANT_SUFFIX}")
    set_property(TARGET "${test_target_name}"
        PROPERTY FOLDER "Tests/check-language")

    set(test_depends_target_name
        "language${test_subset_target_suffix}${test_mode_target_suffix}-test-depends")
    add_custom_target("${test_depends_target_name}"
        DEPENDS "language${test_subset_target_suffix}${test_mode_target_suffix}${LANGUAGE_PRIMARY_VARIANT_SUFFIX}-test-depends")
    set_property(TARGET "${test_depends_target_name}"
        PROPERTY FOLDER "Tests/check-language")
  endforeach()
endforeach()
