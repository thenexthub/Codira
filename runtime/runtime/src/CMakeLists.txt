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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${BOUNDSCHECK_INCLUDE})
if (WINDOWS_FLAG MATCHES 1)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/os/Windows)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Mutator)
endif()

if (BUILD_RUNTIME)
    add_subdirectory(Base)
    add_subdirectory(Common)
    add_subdirectory(Concurrency)
    add_subdirectory(Demangler)
    add_subdirectory(Loader)
    add_subdirectory(Inspector)
    add_subdirectory(Exception)
    add_subdirectory(Heap)
    add_subdirectory(Mutator)
    add_subdirectory(ObjectModel)
    add_subdirectory(UnwindStack)
    add_subdirectory(Sync)
    add_subdirectory(CpuProfiler)
    if (WINDOWS_FLAG MATCHES 0)
        add_subdirectory(Signal)
    endif()
    if(SANITIZER_SUPPORT_FLAG OR GWPASAN_SUPPORT_FLAG)
        add_subdirectory(Sanitizer)
    endif()

    # set(CMAKE_C_FLAGS "-O2 -fPIC -std=c99")
    # set(CMAKE_ASM_FLAGS "-O2 -fPIC -std=gnu++14")

    if ((CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64") AND (MACOS_FLAG MATCHES 1))
    set(SRC_LIST
        "CompilerCalls.cpp"
        "ExceptionManager.cpp"
        "StackManager.cpp"
        "HeapManager.cpp"
        "LoaderManager.cpp"
        "SignalManager.cpp"
        "TypeInfoManager.cpp"
        "LogManager.cpp"
        "CodiraRuntime.cpp"
        "CodeScheduler.cpp"
        "CodeTimer.cpp"
        "CodiraRuntimeApi.cpp"
        "arch/x86_64_macos/N2CStub.S"
        "arch/x86_64_macos/C2NStub.S"
        "arch/x86_64_macos/CalleeSavedStub.S"
        "arch/x86_64_macos/StackGrowStub.S"
        "arch/x86_64_macos/HandleSafepointStub.S"
        "os/Linux/Loader.cpp"
        "os/Linux/Path.cpp"
    )
elseif ((CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" AND IOS_FLAG MATCHES 1) OR IOS_SIMULATOR_FLAG MATCHES 2)
    set(SRC_LIST
        "CompilerCalls.cpp"
        "ExceptionManager.cpp"
        "StackManager.cpp"
        "HeapManager.cpp"
        "LoaderManager.cpp"
        "SignalManager.cpp"
        "TypeInfoManager.cpp"
        "LogManager.cpp"
        "CodiraRuntime.cpp"
        "CodeScheduler.cpp"
        "CodeTimer.cpp"
        "CodiraRuntimeApi.cpp"
        "arch/x86_64_ios/N2CStub.S"
        "arch/x86_64_ios/C2NStub.S"
        "arch/x86_64_ios/CalleeSavedStub.S"
        "arch/x86_64_ios/StackGrowStub.S"
        "arch/x86_64_ios/HandleSafepointStub.S"
        "Os/Linux/Loader.cpp"
        "Os/Linux/Path.cpp"
    )
elseif ((CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64") AND (WINDOWS_FLAG MATCHES 0) AND (IOS_FLAG MATCHES 0) AND (IOS_SIMULATOR_FLAG MATCHES 0))
    set(SRC_LIST
        "CompilerCalls.cpp"
        "ExceptionManager.cpp"
        "StackManager.cpp"
        "HeapManager.cpp"
        "LoaderManager.cpp"
        "SignalManager.cpp"
        "TypeInfoManager.cpp"
        "LogManager.cpp"
        "CodiraRuntime.cpp"
        "CodeScheduler.cpp"
        "CodeTimer.cpp"
        "CodiraRuntimeApi.cpp"
        "arch/x86_64_linux/N2CStub.S"
        "arch/x86_64_linux/C2NStub.S"
        "arch/x86_64_linux/CalleeSavedStub.S"
        "arch/x86_64_linux/StackGrowStub.S"
        "arch/x86_64_linux/HandleSafepointStub.S"
        "os/Linux/Loader.cpp"
        "os/Linux/Path.cpp"
    )
elseif ((CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64") AND (MACOS_FLAG MATCHES 1))
    set(SRC_LIST
        "CompilerCalls.cpp"
        "ExceptionManager.cpp"
        "StackManager.cpp"
        "HeapManager.cpp"
        "LoaderManager.cpp"
        "SignalManager.cpp"
        "TypeInfoManager.cpp"
        "LogManager.cpp"
        "CodiraRuntime.cpp"
        "CodeScheduler.cpp"
        "CodeTimer.cpp"
        "CodiraRuntimeApi.cpp"
        "arch/aarch64_macos/N2CStub.S"
        "arch/aarch64_macos/C2NStub.S"
        "arch/aarch64_macos/CalleeSavedStub.S"
        "arch/aarch64_macos/StackGrowStub.S"
        "arch/aarch64_macos/HandleSafepointStub.S"
        "os/Linux/Loader.cpp"
        "os/Linux/Path.cpp"
    )
elseif ((IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1) AND (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64"))
    set(SRC_LIST
        "CompilerCalls.cpp"
        "ExceptionManager.cpp"
        "StackManager.cpp"
        "HeapManager.cpp"
        "LoaderManager.cpp"
        "SignalManager.cpp"
        "TypeInfoManager.cpp"
        "LogManager.cpp"
        "CodiraRuntime.cpp"
        "CodeScheduler.cpp"
        "CodeTimer.cpp"
        "CodiraRuntimeApi.cpp"
        "arch/aarch64_ios/N2CStub.S"
        "arch/aarch64_ios/C2NStub.S"
        "arch/aarch64_ios/CalleeSavedStub.S"
        "arch/aarch64_ios/StackGrowStub.S"
        "arch/aarch64_ios/HandleSafepointStub.S"
        "Os/Linux/Loader.cpp"
        "Os/Linux/Path.cpp"
    )
elseif ((CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64") AND (WINDOWS_FLAG MATCHES 0))
    set(SRC_LIST
        "CompilerCalls.cpp"
        "ExceptionManager.cpp"
        "StackManager.cpp"
        "HeapManager.cpp"
        "LoaderManager.cpp"
        "SignalManager.cpp"
        "TypeInfoManager.cpp"
        "LogManager.cpp"
        "CodiraRuntime.cpp"
        "CodeScheduler.cpp"
        "CodeTimer.cpp"
        "CodiraRuntimeApi.cpp"
        "arch/aarch64_linux/N2CStub.S"
        "arch/aarch64_linux/C2NStub.S"
        "arch/aarch64_linux/CalleeSavedStub.S"
        "arch/aarch64_linux/StackGrowStub.S"
        "arch/aarch64_linux/HandleSafepointStub.S"
        "os/Linux/Loader.cpp"
        "os/Linux/Path.cpp"
    )
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(SRC_LIST
        "CompilerCalls.cpp"
        "ExceptionManager.cpp"
        "StackManager.cpp"
        "HeapManager.cpp"
        "LoaderManager.cpp"
        "SignalManager.cpp"
        "TypeInfoManager.cpp"
        "LogManager.cpp"
        "CodiraRuntime.cpp"
        "CodeScheduler.cpp"
        "CodeTimer.cpp"
        "CodiraRuntimeApi.cpp"
        "arch/arm_linux/N2CStub.S"
        "arch/arm_linux/C2NStub.S"
        "arch/arm_linux/CalleeSavedStub.S"
        "arch/arm_linux/StackGrowStub.S"
        "arch/arm_linux/HandleSafepointStub.S"
        "os/Linux/Loader.cpp"
        "os/Linux/Path.cpp"
    )
elseif (WINDOWS_FLAG MATCHES 1)
    set(SRC_LIST
        "CompilerCalls.cpp"
        "ExceptionManager.cpp"
        "StackManager.cpp"
        "HeapManager.cpp"
        "LoaderManager.cpp"
        "LogManager.cpp"
        "TypeInfoManager.cpp"
        "CodiraRuntime.cpp"
        "CodeScheduler.cpp"
        "CodeTimer.cpp"
        "CodiraRuntimeApi.cpp"
        "arch/x86_64_windows/N2CStub.S"
        "arch/x86_64_windows/C2NStub.S"
        "arch/x86_64_windows/CalleeSavedStub.S"
        "arch/x86_64_windows/GetContext.S"
        "arch/x86_64_windows/HandleSafepointStub.S"
        "arch/x86_64_windows/ThrowStackOverflowStub.S"
        "arch/x86_64_windows/StackGrowStub.S"
        "os/Windows/Loader.cpp"
        "os/Windows/Path.cpp"
        "os/Windows/UnwindWin.cpp"
        "os/Windows/WinModuleManager.cpp"
    )
endif()
    add_library(runtime STATIC ${SRC_LIST})
    add_library(cangjie-runtime SHARED ${SRC_LIST})

elseif (BUILD_DEMANGLE)
    add_subdirectory(Demangler)
endif() # BUILD_RUNTIME
