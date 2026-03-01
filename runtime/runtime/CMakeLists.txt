# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

cmake_minimum_required(VERSION 3.16.3)
if (OHOS_FLAG MATCHES 1 OR OHOS_FLAG MATCHES 2 OR OHOS_FLAG MATCHES 3)
    # Target OHOS is Linux-like system.
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)
endif()

if (ANDROID_FLAG MATCHES 1 OR ANDROID_FLAG MATCHES 2)
    if (CMAKE_ANDROID_API MATCHES 26)
        set(ANDROID_TARGET_API "android26")
    elseif (CMAKE_ANDROID_API MATCHES 31)
        set(ANDROID_TARGET_API "android31")
    else ()
        message(FATAL_ERROR "ANDROID_TARGET_API is not set, please check the target option.")
    endif()
    set(ANDROID_OS "linux")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)
    if (ANDROID_FLAG MATCHES 1)
        set(ANDROID_ARCH "aarch64")
    else ()
        set(ANDROID_ARCH "x86_64")
    endif()
endif()

if (WINDOWS_FLAG MATCHES 1)
    set(CMAKE_SYSTEM_NAME Windows)
endif()
# Windows target must be set before project(CodiraRuntime).
if (WINDOWS_FLAG MATCHES 1)
    # Use target x86_64-w64-mingw32 instead of x86_64-windows-gnu
    # to make sure clang can find x86_64-w64-mingw32-ld (linker for cross-compiling)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -target x86_64-w64-mingw32 -pthread -femulated-tls --rtlib=compiler-rt")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -target x86_64-w64-mingw32 -pthread -femulated-tls -stdlib=libc++ --rtlib=compiler-rt")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -static -target x86_64-w64-mingw32 --rtlib=compiler-rt")
    if (NOT CMAKE_AR_PATH)
        set(CMAKE_AR_PATH "llvm-ar.exe")
    endif()
endif ()

# project name: CodiraRuntime
project(CodiraRuntime C CXX ASM)
include(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/SetupAr.cmake)

message("-- BUILD_CODETHREAD: " ${BUILD_CODETHREAD})
message("-- BUILD_RUNTIME: "   ${BUILD_RUNTIME})

if (SANITIZER_SUPPORT_FLAG MATCHES 1)
    add_compile_definitions(CODIRA_SANITIZER_SUPPORT)
endif()

if (SANITIZER_SUPPORT_FLAG MATCHES 1 OR GWPASAN_SUPPORT_FLAG MATCHES 1)
    set(SANITIZER_LIB "-Wl,--whole-archive" "Sanitizer" "-Wl,--no-whole-archive")
    set(SANITIZER_LIB_STATIC "$<TARGET_OBJECTS:Sanitizer>")
endif ()

set(CODETHREAD_SAN_PRARM "code")
set(SANITIZER_SUBPATH "")
if (ASAN_SUPPORT_FLAG MATCHES 1)
    set(CODETHREAD_SAN_PRARM "codeasan")
    add_compile_definitions(CODIRA_ASAN_SUPPORT)
    add_compile_definitions(GENERAL_ASAN_SUPPORT_INTERFACE)
    set(SANITIZER_SUBPATH /asan)
elseif (TSAN_SUPPORT_FLAG MATCHES 1)
    set(CODETHREAD_SAN_PRARM "codetsan")
    add_compile_definitions(CODIRA_TSAN_SUPPORT)
    set(SANITIZER_SUBPATH /tsan)
elseif (HWASAN_SUPPORT_FLAG MATCHES 1)
    set(CODETHREAD_SAN_PRARM "codehwasan")
    add_compile_definitions(CODIRA_HWASAN_SUPPORT)
    add_compile_definitions(GENERAL_ASAN_SUPPORT_INTERFACE)
    set(SANITIZER_SUBPATH /hwasan)
elseif (GWPASAN_SUPPORT_FLAG MATCHES 1)
    add_compile_definitions(CODIRA_GWPASAN_SUPPORT)
endif ()

set(OTHER_DEFINITIONS " ")
if (RUNTIME_FORWARD_PTRAUTH_CFI MATCHES 1)
    set(OTHER_DEFINITIONS "${OTHER_DEFINITIONS} -DRUNTIME_FORWARD_PTRAUTH_CFI=1")
    add_compile_definitions(ENABLE_FORWARD_PTRAUTH_CFI)
endif ()
if (RUNTIME_BACKWARD_PTRAUTH_CFI MATCHES 1)
    set(OTHER_DEFINITIONS "${OTHER_DEFINITIONS} -DRUNTIME_BACKWARD_PTRAUTH_CFI=1")
    add_compile_definitions(ENABLE_BACKWARD_PTRAUTH_CFI)
endif ()
if (ANDROID_FLAG MATCHES 1 OR ANDROID_FLAG MATCHES 2)
    set(OTHER_DEFINITIONS "-DCMAKE_C_COMPILER_FORCED=ON -DCMAKE_CXX_COMPILER_FORCED=ON")
endif ()

if (BUILD_DEMANGLE)
    message("-- BUILD_DEMANGLE: " ${BUILD_DEMANGLE})
    add_subdirectory(src)
endif() # BUILD_DEMANGLE

if (BUILD_CODETHREAD) # BUILD_CODETHREAD
# build codethread
if (WINDOWS_FLAG MATCHES 1)
    if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        message("build codethread for windows on windows")
        # build\build_codethread_windows.bat -p windows_x86_64 Debug STATIC
        execute_process(COMMAND build\\build_codethread_windows.bat -p windows_x86_64 ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} "${OTHER_DEFINITIONS}" "${CMAKE_INSTALL_PREFIX}" ${TARGET_ARCH})
    elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
        message("build codethread for windows on linux")
        execute_process(COMMAND bash build/build_codethread.sh -p cross_windows_x86_64 ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} "${OTHER_DEFINITIONS}" "${CMAKE_INSTALL_PREFIX}" ${TARGET_ARCH})
    else ()
        message("Only the Windows and Linux platforms are supported.")
    endif ()
elseif (OHOS_FLAG MATCHES 1) # for ohos-aarch64
    if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        message("build codethread for ohos on windows")
        execute_process(COMMAND build\\build_codethread_windows.bat -p ohos_aarch64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    else ()
        message("build codethread for ohos")
        execute_process(COMMAND bash build/build_codethread.sh -p ohos_aarch64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    endif ()
elseif (OHOS_FLAG MATCHES 2) # for ohos-x86_64
    if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        message("build codethread for ohos on windows")
        execute_process(COMMAND build\\build_codethread_windows.bat -p ohos_x86_64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    else ()
        message("build codethread for ohos")
        execute_process(COMMAND bash build/build_codethread.sh -p ohos_x86_64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    endif ()
elseif (OHOS_FLAG MATCHES 3) # for ohos-arm
    if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        message("build codethread for ohos on windows")
        execute_process(COMMAND build\\build_codethread_windows.bat -p ohos_arm_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    else ()
        message("build codethread for ohos")
        execute_process(COMMAND bash build/build_codethread.sh -p ohos_arm_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    endif ()
elseif (ANDROID_FLAG MATCHES 1)
    message("build codethread for ${ANDROID_TARGET_API}")
    set(ENV{ANDROID_ROOT} "${ANDROID_ROOT}")
    set(ENV{ANDROID_ARCH} "${ANDROID_ARCH}")
    set(ENV{ANDROID_TARGET_API} "${ANDROID_TARGET_API}")
    execute_process(COMMAND sh build/build_codethread.sh -p android_aarch64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
elseif (ANDROID_FLAG MATCHES 2)
    message("build codethread for ${ANDROID_TARGET_API}")
    set(ENV{ANDROID_ROOT} "${ANDROID_ROOT}")
    set(ENV{ANDROID_ARCH} "${ANDROID_ARCH}")
    set(ENV{ANDROID_TARGET_API} "${ANDROID_TARGET_API}")
    execute_process(COMMAND sh build/build_codethread.sh -p android_x86_64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
elseif (MACOS_FLAG MATCHES 1)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        message("build codethread for macos x86_64")
        execute_process(COMMAND bash build/build_codethread.sh -p macos_x86_64 ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    else()
        message("build codethread for macos aarch64")
        execute_process(COMMAND bash build/build_codethread.sh -p macos_aarch64 ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    endif()
elseif (IOS_FLAG MATCHES 1)
    message("build codethread for ios")
    execute_process(COMMAND sh build/build_codethread.sh -p ios_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
elseif (IOS_SIMULATOR_FLAG MATCHES 2)
        message("build codethread for ios simulator x86_64")
        execute_process(COMMAND sh build/build_codethread.sh -p ios_simulator_x86_64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
elseif (IOS_SIMULATOR_FLAG MATCHES 1)
        message("build codethread for ios simulator aarch64")
        execute_process(COMMAND sh build/build_codethread.sh -p ios_simulator_aarch64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else ()
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        message("build codethread for aarch64")
        # bash build/build_codethread.sh -p linux_aarch64_cangjie Debug STATIC ${CODETHREAD_SAN_PRARM}
        execute_process(COMMAND sh build/build_codethread.sh -p linux_aarch64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    else ()
        # bash build/build_codethread.sh -p linux_x86_64_cangjie Debug STATIC ${CODETHREAD_SAN_PRARM}
        message("build codethread for x86_64")
        execute_process(COMMAND bash build/build_codethread.sh -p linux_x86_64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH})
    endif ()
endif () # WINDOWS_FLAG

message("build codethread done!!!")
endif() # BUILD_CODETHREAD

if (BUILD_RUNTIME)
include_directories(${DOPRA_INCLUDE})

if (DOPRA_FLAG MATCHES 1)
    add_definitions(-DMRT_USE_CODETHREAD)
    add_definitions(-DMRT_USE_CODETHREAD_RENAME)
endif ()
if (MACOS_FLAG MATCHES 1 OR IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2)
    add_definitions(-D_XOPEN_SOURCE=600)
    add_definitions(-D_DARWIN_C_SOURCE)
    # add_definitions(-D__USE_GNU)
endif()

# Build the runtime for ARM-based Euler operating system
# the -outline-atomics option is enabled by default, which can cause symbol not found.
# To resolve the issue, disable the option by default.
if (EULER_FLAG MATCHES 1 AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" AND WINDOWS_FLAG MATCHES 1)
    message("disable option -mno-outline-atomics")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-outline-atomics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-outline-atomics")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mno-outline-atomics")
endif ()

if (COPYGC_FLAG MATCHES 1)
    add_definitions(-DMRT_USE_COPYGC)
endif ()

if (DUMPADDRESS_FLAG MATCHES 1)
    add_definitions(-DMRT_DUMP_ADDRESS)
endif ()

if (CODE_SDK_VERSION)
    add_definitions(-DCODE_SDK_VERSION="${CODE_SDK_VERSION}")
endif()

if (DISABLE_VERSION_CHECK MATCHES 1)
    add_definitions(-DDISABLE_VERSION_CHECK)
endif()

# add singal handle function for the coverage calculation
if (COV MATCHES 1)
    add_definitions(-DCOV_SIGNALHANDLE)
endif ()

link_directories(${DOPRA_LIBS})
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
if (ANDROID_FLAG MATCHES 1)
    FILE(GLOB_RECURSE CLANG_LIB_FILES ${ANDROID_ROOT}/libclang_rt.builtins-aarch64-android.a)
    list(GET CLANG_LIB_FILES 0 FIRST_FILE)
    get_filename_component(CLANG_LIB_PARENT_DIR ${FIRST_FILE} DIRECTORY)
    link_directories(${CLANG_LIB_PARENT_DIR})
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
elseif (ANDROID_FLAG MATCHES 2)
    FILE(GLOB_RECURSE CLANG_LIB_FILES ${ANDROID_ROOT}/libclang_rt.builtins-x86_64-android.a)
    list(GET CLANG_LIB_FILES 0 FIRST_FILE)
    get_filename_component(CLANG_LIB_PARENT_DIR ${FIRST_FILE} DIRECTORY)
    link_directories(${CLANG_LIB_PARENT_DIR})
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
elseif (MACOS_FLAG MATCHES 1)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE output OUTPUT_STRIP_TRAILING_WHITESPACE)
    # set(output "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk")
    link_directories(${output}/usr/lib)
    link_directories(/usr/local/lib)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0.0)
elseif (IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2)
    set(IOS_PLATFORM "SIMULATOR")
    set(IOS_PLATFORM_LOCATION "iPhoneSimulator.platform")
    set(CMAKE_IOS_DEVELOPER_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer")
    set(CMAKE_IOS_SDK_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator17.5.sdk")
    set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator17.5.sdk")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "")
elseif (IOS_FLAG MATCHES 1)
    set(IOS ON)
    set(IOS_PLATFORM OS)
    set(IOS_PLATFORM_LOCATION "iPhoneOS.platform")
    set(CMAKE_IOS_DEVELOPER_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/${IOS_PLATFORM_LOCATION}/Developer")
    set(CMAKE_IOS_SDK_ROOT "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneOS17.5.sdk")
    set(CMAKE_OSX_SYSROOT "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneOS17.5.sdk")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "")
endif()

link_directories(${OHOS_LIB})
include_directories(${BOUNDSCHECK_INCLUDE})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(third_party/third_party_bounds_checking_function)
if (ANDROID_FLAG MATCHES 0 AND OHOS_FLAG MATCHES 0 AND WINDOWS_FLAG MATCHES 0 AND MACOS_FLAG MATCHES 0 AND IOS_FLAG MATCHES 0 AND IOS_SIMULATOR_FLAG MATCHES 0)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(LINK_SCRIPTS "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/build/lds/aarch64_linux/codenative_runtime.lds")
    add_library(codeNativeStartAndEnd OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/aarch64_linux/CodeNativeRuntimeStartAndEnd.S)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(LINK_SCRIPTS "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/build/lds/x86_64_linux/codenative_runtime.lds")
    add_library(codeNativeStartAndEnd OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64_linux/CodeNativeRuntimeStartAndEnd.S)
endif()
endif()

if (MACOS_FLAG MATCHES 1 OR IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2)
    set(COMMON_LINK_FLAGS "-rdynamic")
else()
    set(COMMON_LINK_FLAGS "-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,--gc-sections -rdynamic ${LINK_SCRIPTS}")
endif()

if (COV MATCHES 1)
    set_target_properties(cangjie-runtime PROPERTIES LINK_FLAGS "${COMMON_LINK_FLAGS} ${CMAKE_COV_FLAGS}")
    target_compile_definitions(cangjie-runtime PUBLIC COMPILE_DYNAMIC)
    target_compile_options(cangjie-runtime PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Base PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(CodeFile PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Common PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Concurrency PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Exception PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Heap PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Allocator PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Barrier PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Collector PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Loader PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Mutator PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(ObjectModel PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Inspector PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Signal PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(BinaryFile PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(CodeFileLoader PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(UnwindStack PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(Sync PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(WCollector PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(CodeSemanticVersion PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    target_compile_options(CpuProfiler PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    if (DOPRA_FLAG MATCHES 1)
        target_compile_options(codethreadmodel PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    elseif (DOPRA_FLAG MATCHES 0)
        target_compile_options(threadmodel PUBLIC "SHELL:${CMAKE_COV_FLAGS}")
    endif ()
else ()
    target_compile_definitions(cangjie-runtime PUBLIC COMPILE_DYNAMIC)
    if (WINDOWS_FLAG MATCHES 1)
        set_target_properties(cangjie-runtime PROPERTIES LINK_FLAGS " -Wl,--gc-sections")
    elseif (WINDOWS_FLAG MATCHES 0)
        set_target_properties(cangjie-runtime PROPERTIES LINK_FLAGS "${COMMON_LINK_FLAGS}")
        target_compile_definitions(cangjie-runtime PUBLIC COMPILE_DYNAMIC)
    endif ()
endif ()

# Difference in Windows: remove signal module
# Signal processing use sigaction() as signal-catching function currently, it`s linux related, which can get context when signal was triggered. In Windows, signal() is the only function we can use, but there is no context.
if (WINDOWS_FLAG MATCHES 1)
    list(APPEND LINK_LIBS "boundscheck" "Base" "Common" "Concurrency" "Loader" "Inspector" "Exception" "Heap" "Mutator" "ObjectModel" "UnwindStack"
    "Allocator" "Barrier" "Collector" "WCollector" "BinaryFile" "CodeFile" "CodeFileLoader" "CodeSemanticVersion" "Sync" "dbghelp" "ws2_32" "shlwapi" "CpuProfiler" ${SANITIZER_LIB})
elseif (WINDOWS_FLAG MATCHES 0)
    list(APPEND LINK_LIBS "boundscheck" "Base" "Common" "Concurrency" "Loader" "Inspector" "Exception" "Heap" "Mutator" "ObjectModel" "UnwindStack" "Signal"
    "Allocator" "Barrier" "Collector" "WCollector" "BinaryFile" "CodeFile" "CodeFileLoader" "CodeSemanticVersion" "Sync" "CpuProfiler" ${SANITIZER_LIB})
endif()

if (DOPRA_FLAG MATCHES 1)
    if (MACOS_FLAG MATCHES 1 OR IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2)
        list(APPEND LINK_LIBS "codethreadmodel" "cangjie-thread")
    else()
        list(APPEND LINK_LIBS "codethreadmodel" "-Wl,--whole-archive" "cangjie-thread" "-Wl,--no-whole-archive")
    endif()
elseif (DOPRA_FLAG MATCHES 0)
    list(APPEND LINK_LIBS "threadmodel")
endif ()

if (ASAN_FLAG MATCHES 1)
    list(APPEND LINK_LIBS "-fsanitize=address")
endif()

if (HWASAN_FLAG MATCHES 1)
    list(APPEND LINK_LIBS "-shared-libsan -fsanitize=hwaddress -fno-emulated-tls -fno-lto -fno-sanitize=cfi -mllvm -hwasan-globals=0 -fno-whole-program-vtables")
endif()

if (ANDROID_FLAG MATCHES 1)
    list(APPEND LINK_LIBS "clang_rt.builtins-aarch64-android")
endif()
if (ANDROID_FLAG MATCHES 2)
    list(APPEND LINK_LIBS "clang_rt.builtins-x86_64-android")
endif()

if (DOPRA_FLAG MATCHES 1 AND OHOS_FLAG MATCHES 0 AND WINDOWS_FLAG MATCHES 0) # linux
    if (COV MATCHES 1) # linux - cov
        target_link_libraries(cangjie-runtime "-Wl,--start-group -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/local_export.map"
            "-lstdc++ -lpthread -ldl" ${LINK_LIBS} "codeNativeStartAndEnd" "-Wl,--end-group")
    else ()
        if (ANDROID_FLAG MATCHES 1 OR ANDROID_FLAG MATCHES 2) # android
            target_link_libraries(cangjie-runtime
                "-Wl,--start-group -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/local_export.map"
                ${LINK_LIBS} "-Wl,--end-group -nodefaultlibs -Wl,-Bdynamic -lc++ -lunwind -lm -lc -ldl -landroid -llog")
        elseif (MACOS_FLAG MATCHES 1 OR IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2) # macos,ios
            target_link_libraries(cangjie-runtime ${LINK_LIBS} "-Wl,-no_compact_unwind -nodefaultlibs -lc++ -lm -lc -lpthread -ldl")
        else() # normal
            target_link_libraries(cangjie-runtime "-Wl,--start-group -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/${EXPORT_MAP}"
                ${LINK_LIBS} "codeNativeStartAndEnd" "-Wl,--end-group -nodefaultlibs -Wl,-Bdynamic -lstdc++ -lm -lc -lgcc_s -lpthread -ldl")
        endif()
    endif ()
elseif (DOPRA_FLAG MATCHES 1 AND OHOS_FLAG IN_LIST OHOS_FLAG_LIST AND WINDOWS_FLAG MATCHES 0) # OHOS
    target_link_libraries(cangjie-runtime "-Wl,--start-group -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/${EXPORT_MAP}"
        "-lstdc++" ${LINK_LIBS} "-Wl,--end-group -lunwind -lclang_rt.builtins -lhilog_ndk.z -lhitrace_ndk.z")
elseif (DOPRA_FLAG MATCHES 1 AND OHOS_FLAG MATCHES 0 AND WINDOWS_FLAG MATCHES 1) # Windows
    target_link_libraries(cangjie-runtime PRIVATE "-Wl,--start-group"
        "-Wl,--export-all-symbols" "-lc++" "-lunwind" ${LINK_LIBS} "-Wl,--end-group -Wl,--no-insert-timestamp")
elseif (DOPRA_FLAG MATCHES 0 AND WINDOWS_FLAG MATCHES 0) # thread module
    target_link_libraries(cangjie-runtime "-Wl,--start-group -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/${EXPORT_MAP}"
        ${LINK_LIBS} "-Wl,--end-group -nodefaultlibs -Wl,-Bdynamic -lstdc++ -lm -lc -lgcc_s -lpthread -ldl")
elseif (DOPRA_FLAG MATCHES 0 AND WINDOWS_FLAG MATCHES 1) # thread module on windows
    target_link_libraries(cangjie-runtime "-Wl,--start-group -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/local_export.map"
        ${LINK_LIBS} "-Wl,--end-group -nodefaultlibs -Wl,-Bdynamic -lstdc++ -lc -lgcc_s -Wl,--no-insert-timestamp")
endif ()


# setting for codethread trace
if (DOPRA_FLAG MATCHES 1 AND RUNTIME_TRACE_FLAG MATCHES 1)
    file(WRITE ${CMAKE_BINARY_DIR}/null.c "")
    if (MACOS_FLAG MATCHES 1 OR IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2)
        link_libraries("-Wl" "codethread-trace" "-Wl" "boundscheck")
    elseif(WINDOWS_FLAG MATCHES 1)
        link_libraries("-Wl,--whole-archive" "codethread-trace" "-Wl,--no-whole-archive" "boundscheck")
    else()
        link_libraries("-Wl,--whole-archive" "codethread-trace" "-Wl,--no-whole-archive" "boundscheck" "-Wl,-z,relro -Wl,-z,now")
    endif()
    add_library(cangjie-trace SHARED null.c)
    if (WINDOWS_FLAG MATCHES 1)
        target_link_libraries(cangjie-trace cangjie-runtime)
        target_link_options(cangjie-trace PRIVATE "-Wl,--no-insert-timestamp")
    endif()
    install(TARGETS cangjie-trace DESTINATION runtime/lib/${TARGET_ARCH}${SANITIZER_SUBPATH})
endif()

install(TARGETS cangjie-runtime DESTINATION runtime/lib/${TARGET_ARCH}${SANITIZER_SUBPATH})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/RuntimeConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Codira.h
        DESTINATION include)

# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/envsetup.sh DESTINATION ${CMAKE_INSTALL_PREFIX})

if (WINDOWS_FLAG MATCHES 1)
    add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64_windows/Codestart.S)
    add_library(section OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/os/Windows/Section.cpp)
elseif (MACOS_FLAG MATCHES 1)
    add_library(section OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/os/Macos/section.cpp)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/X86_64_macos/Codestart.S)
    else()
        add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/Aarch64_macos/Codestart.S)
    endif()
elseif (IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2)
    add_library(section OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/os/Macos/Section.cpp)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64_ios/Codestart.S)
    else()
        if (IOS_SIMULATOR_FLAG MATCHES 2)
            add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64_ios/Codestart.S)
        else ()
            add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/aarch64_ios/Codestart.S)
        endif()
    endif()
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/aarch64_linux/Codestart.S)
    install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/aarch64_linux/codeld.lds
            ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/aarch64_linux/discard_eh_frame.lds
            ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/aarch64_linux/cld.lds
            ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/aarch64_linux/codeld.shared.lds
            DESTINATION lib/${TARGET_ARCH})
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/arm_linux/Codestart.S)
    install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/arm_linux/codeld.lds
        ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/arm_linux/discard_eh_frame.lds
        ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/arm_linux/cld.lds
        ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/arm_linux/codeld.shared.lds
        DESTINATION lib/${TARGET_ARCH})
else()
    add_library(codestart OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64_linux/Codestart.S)
    install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/x86_64_linux/codeld.lds
            ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/x86_64_linux/discard_eh_frame.lds
            ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/x86_64_linux/cld.lds
            ${CMAKE_CURRENT_SOURCE_DIR}/build/lds/x86_64_linux/codeld.shared.lds
            DESTINATION lib/${TARGET_ARCH})
endif()
install(PROGRAMS $<TARGET_OBJECTS:codestart>
        RENAME codestart.o
        DESTINATION lib/${TARGET_ARCH})
if (MACOS_FLAG MATCHES 1 OR WINDOWS_FLAG MATCHES 1 OR IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2)
    install(PROGRAMS $<TARGET_OBJECTS:section>
        RENAME section.o
        DESTINATION lib/${TARGET_ARCH})
endif()

if (CMAKE_BUILD_TYPE MATCHES "^(Release|MinSizeRel)$")
    message("strip: ${CMAKE_INSTALL_PREFIX}")
    set(STRIP_PROGRAM strip)
    if(OHOS_FLAG IN_LIST OHOS_FLAG_LIST)
        # System installed `strip` may not support aarch64 targets. We use llvm-strip in OHOS toolchain.
        if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
            set(STRIP_PROGRAM $ENV{OHOS_ROOT}/prebuilts/clang/ohos/windows-x86_64/llvm/bin/llvm-strip)
        endif()
        if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
            set(STRIP_PROGRAM $ENV{OHOS_ROOT}/prebuilts/clang/ohos/linux-x86_64/llvm/bin/llvm-strip)
        endif()
        if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
            set(STRIP_PROGRAM $ENV{OHOS_ROOT}/prebuilts/clang/ohos/${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_HOST_SYSTEM_PROCESSOR}/llvm/bin/llvm-strip)
        endif()
    endif()
    if (ANDROID_FLAG MATCHES 1 OR ANDROID_FLAG MATCHES 2)
        if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
            set(STRIP_PROGRAM $ENV{ANDROID_ROOT}/llvm/prebuilt/darwin-x86_64/bin/llvm-strip)
        else()
            set(STRIP_PROGRAM $ENV{ANDROID_ROOT}/llvm/prebuilt/linux-x86_64/bin/llvm-strip)
        endif()
    endif()
    if (WINDOWS_FLAG MATCHES 1)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} -p ${CMAKE_INSTALL_PREFIX}/runtime/lib/${TARGET_ARCH}/libcangjie-runtime.dll)"
            COMPONENT cangjie-runtime)
        if (ASAN_SUPPORT_FLAG MATCHES 0)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} ${CMAKE_INSTALL_PREFIX}/bin/codefilt.exe)"
            COMPONENT codefilt)
        endif()
        if (RUNTIME_TRACE_FLAG MATCHES 1)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} -p ${CMAKE_INSTALL_PREFIX}/runtime/lib/${TARGET_ARCH}/libcangjie-trace.dll)"
            COMPONENT cangjie-trace)
        endif()
    elseif (MACOS_FLAG MATCHES 1 OR IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} -x ${CMAKE_INSTALL_PREFIX}/runtime/lib/${TARGET_ARCH}/libcangjie-runtime.dylib)"
                COMPONENT cangjie-runtime)
        if (ASAN_SUPPORT_FLAG MATCHES 0)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} -u -r ${CMAKE_INSTALL_PREFIX}/bin/codefilt)"
                COMPONENT codefilt)
        endif()
        if (RUNTIME_TRACE_FLAG MATCHES 1)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} ${CMAKE_INSTALL_PREFIX}/runtime/lib/${TARGET_ARCH}/libcangjie-trace.dylib)"
            COMPONENT cangjie-trace)
        endif()
    else()
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} ${CMAKE_INSTALL_PREFIX}/runtime/lib/${TARGET_ARCH}${SANITIZER_SUBPATH}/libcangjie-runtime.so)"
                COMPONENT cangjie-runtime)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} -g ${CMAKE_INSTALL_PREFIX}/lib/${TARGET_ARCH}/codestart.o)"
                COMPONENT codestart)
        if (OHOS_FLAG IN_LIST OHOS_FLAG_LIST)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} ${CMAKE_INSTALL_PREFIX}/runtime/lib/${TARGET_ARCH}/libcangjie-demangle.so)"
                COMPONENT cangjie-demangle)
        endif()
        if (ASAN_SUPPORT_FLAG MATCHES 0)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} ${CMAKE_INSTALL_PREFIX}/bin/codefilt)"
                COMPONENT codefilt)
        endif()
        if (RUNTIME_TRACE_FLAG MATCHES 1)
        install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} ${CMAKE_INSTALL_PREFIX}/runtime/lib/${TARGET_ARCH}${SANITIZER_SUBPATH}/libcangjie-trace.so)"
            COMPONENT cangjie-trace)
        endif()
    endif()
endif () # CMAKE_BUILD_TYPE MATCHES "^(Release|MinSizeRel)$"


if (IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2 OR WINDOWS_FLAG MATCHES 1
    OR MACOS_FLAG MATCHES 1 OR (ANDROID_FLAG MATCHES 0 AND OHOS_FLAG MATCHES 0))
    add_library(cangjie-runtime-static STATIC)
    if (WINDOWS_FLAG MATCHES 1)
        target_sources(cangjie-runtime-static PRIVATE
            $<TARGET_OBJECTS:runtime>
            $<TARGET_OBJECTS:Base>
            $<TARGET_OBJECTS:CodeFile>
            $<TARGET_OBJECTS:Concurrency>
            $<TARGET_OBJECTS:Common>
            $<TARGET_OBJECTS:Exception>
            $<TARGET_OBJECTS:Heap>
            $<TARGET_OBJECTS:Allocator>
            $<TARGET_OBJECTS:Barrier>
            $<TARGET_OBJECTS:Collector>
            $<TARGET_OBJECTS:Loader>
            $<TARGET_OBJECTS:Inspector>
            $<TARGET_OBJECTS:Mutator>
            $<TARGET_OBJECTS:ObjectModel>
            $<TARGET_OBJECTS:BinaryFile>
            $<TARGET_OBJECTS:CodeFileLoader>
            $<TARGET_OBJECTS:UnwindStack>
            $<TARGET_OBJECTS:Sync>
            $<TARGET_OBJECTS:WCollector>
            $<TARGET_OBJECTS:codethreadmodel>
            $<TARGET_OBJECTS:CodeSemanticVersion>
            $<TARGET_OBJECTS:CpuProfiler>
            ${SANITIZER_LIB_STATIC}
        )
    else()
        target_sources(cangjie-runtime-static PRIVATE
            $<TARGET_OBJECTS:runtime>
            $<TARGET_OBJECTS:Base>
            $<TARGET_OBJECTS:CodeFile>
            $<TARGET_OBJECTS:Concurrency>
            $<TARGET_OBJECTS:Common>
            $<TARGET_OBJECTS:Exception>
            $<TARGET_OBJECTS:Heap>
            $<TARGET_OBJECTS:Allocator>
            $<TARGET_OBJECTS:Barrier>
            $<TARGET_OBJECTS:Collector>
            $<TARGET_OBJECTS:Loader>
            $<TARGET_OBJECTS:Inspector>
            $<TARGET_OBJECTS:Mutator>
            $<TARGET_OBJECTS:ObjectModel>
            $<TARGET_OBJECTS:Signal>
            $<TARGET_OBJECTS:BinaryFile>
            $<TARGET_OBJECTS:CodeFileLoader>
            $<TARGET_OBJECTS:UnwindStack>
            $<TARGET_OBJECTS:Sync>
            $<TARGET_OBJECTS:WCollector>
            $<TARGET_OBJECTS:codethreadmodel>
            $<TARGET_OBJECTS:CodeSemanticVersion>
            $<TARGET_OBJECTS:CpuProfiler>
            ${SANITIZER_LIB_STATIC}
        )
    endif()
    if (IOS_FLAG MATCHES 1)
        add_custom_command(TARGET cangjie-runtime-static POST_BUILD
            COMMAND sh build/build_codethread.sh -p ios_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} "apple_static_lib" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
    elseif (IOS_SIMULATOR_FLAG MATCHES 2)
        add_custom_command(TARGET cangjie-runtime-static POST_BUILD
            COMMAND sh ${CMAKE_SOURCE_DIR}/build/build_codethread.sh -p ios_simulator_x86_64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} "apple_static_lib" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
    elseif (IOS_SIMULATOR_FLAG MATCHES 1)
        add_custom_command(TARGET cangjie-runtime-static POST_BUILD
            COMMAND sh ${CMAKE_SOURCE_DIR}/build/build_codethread.sh -p ios_simulator_aarch64_cangjie ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} "apple_static_lib" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
    elseif (MACOS_FLAG MATCHES 1)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            add_custom_command(TARGET cangjie-runtime-static POST_BUILD
                COMMAND sh ${CMAKE_SOURCE_DIR}/build/build_codethread.sh -p macos_x86_64 ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} "apple_static_lib" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                VERBATIM
            )
        else()
            add_custom_command(TARGET cangjie-runtime-static POST_BUILD
                COMMAND sh ${CMAKE_SOURCE_DIR}/build/build_codethread.sh -p macos_aarch64 ${CMAKE_BUILD_TYPE} STATIC ${CODETHREAD_SAN_PRARM} ${OTHER_DEFINITIONS} ${CMAKE_INSTALL_PREFIX} ${TARGET_ARCH} "apple_static_lib" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                VERBATIM
            )
        endif()
    endif()
    if (IOS_FLAG MATCHES 1)
        set(RENAME_PLATFORM "ios_cangjie")
    elseif (IOS_SIMULATOR_FLAG MATCHES 2)
        set(RENAME_PLATFORM "ios_simulator_x86_64_cangjie")
    elseif (IOS_SIMULATOR_FLAG MATCHES 1)
        set(RENAME_PLATFORM "ios_simulator_aarch64_cangjie")
    elseif (MACOS_FLAG MATCHES 1)
        set(RENAME_PLATFORM "macos_cangjie")
    else()
        set(RENAME_PLATFORM "empty")
    endif()
    if (IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 2 OR MACOS_FLAG MATCHES 1)
        add_custom_command(TARGET cangjie-runtime-static PRE_BUILD
            COMMAND sh ${CMAKE_SOURCE_DIR}/build/apple_rename_section.sh
                "${RENAME_PLATFORM}"
                "${CMAKE_C_COMPILER}"
                "$<TARGET_OBJECTS:runtime>"
                "$<TARGET_OBJECTS:Base>"
                "$<TARGET_OBJECTS:CodeFile>"
                "$<TARGET_OBJECTS:Concurrency>"
                "$<TARGET_OBJECTS:Common>"
                "$<TARGET_OBJECTS:Exception>"
                "$<TARGET_OBJECTS:Heap>"
                "$<TARGET_OBJECTS:Allocator>"
                "$<TARGET_OBJECTS:Barrier>"
                "$<TARGET_OBJECTS:Collector>"
                "$<TARGET_OBJECTS:Loader>"
                "$<TARGET_OBJECTS:Inspector>"
                "$<TARGET_OBJECTS:Mutator>"
                "$<TARGET_OBJECTS:ObjectModel>"
                "$<TARGET_OBJECTS:Signal>"
                "$<TARGET_OBJECTS:BinaryFile>"
                "$<TARGET_OBJECTS:CodeFileLoader>"
                "$<TARGET_OBJECTS:UnwindStack>"
                "$<TARGET_OBJECTS:Sync>"
                "$<TARGET_OBJECTS:WCollector>"
                "$<TARGET_OBJECTS:codethreadmodel>"
                "$<TARGET_OBJECTS:CodeSemanticVersion>"
                "$<TARGET_OBJECTS:CpuProfiler>"
                "${SANITIZER_LIB_STATIC}"
            VERBATIM
        )
    endif()
install(TARGETS cangjie-runtime-static ARCHIVE DESTINATION lib/${TARGET_ARCH}${SANITIZER_SUBPATH})
set_target_properties(cangjie-runtime-static PROPERTIES OUTPUT_NAME "cangjie-runtime")

set(OBJCOPY_PROGRAM objcopy)
set(STRIP_PROGRAM strip)

install(CODE "execute_process(COMMAND ${OBJCOPY_PROGRAM} -D --rename-section .text=.text_runtime \
            ${CMAKE_INSTALL_PREFIX}/lib/${TARGET_ARCH}${SANITIZER_SUBPATH}/libcangjie-runtime.a)"
        COMPONENT cangjie-runtime)
if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} -D --strip-unneeded ${CMAKE_INSTALL_PREFIX}/lib${SANITIZER_SUBPATH}/libcangjie-runtime.a)"
        COMPONENT cangjie-runtime)
endif()
endif()

endif() # BUILD_RUNTIME
