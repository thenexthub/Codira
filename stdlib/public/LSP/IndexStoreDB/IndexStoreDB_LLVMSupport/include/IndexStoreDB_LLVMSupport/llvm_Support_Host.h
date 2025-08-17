//===- toolchain/Support/Host.h - Host machine characteristics --------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// Methods for querying the nature of the host machine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_HOST_H
#define LLVM_SUPPORT_HOST_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringMap.h>

#if defined(__linux__) || defined(__GNU__) || defined(__HAIKU__)
#include <endian.h>
#elif defined(_AIX)
#include <sys/machine.h>
#elif defined(__sun)
/* Solaris provides _BIG_ENDIAN/_LITTLE_ENDIAN selector in sys/types.h */
#include <sys/types.h>
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#if defined(_BIG_ENDIAN)
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#else
#if !defined(BYTE_ORDER) && !defined(_WIN32)
#include <machine/endian.h>
#endif
#endif

#include <string>

namespace toolchain {
namespace sys {

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
constexpr bool IsBigEndianHost = true;
#else
constexpr bool IsBigEndianHost = false;
#endif

  static const bool IsLittleEndianHost = !IsBigEndianHost;

  /// getDefaultTargetTriple() - Return the default target triple the compiler
  /// has been configured to produce code for.
  ///
  /// The target triple is a string in the format of:
  ///   CPU_TYPE-VENDOR-OPERATING_SYSTEM
  /// or
  ///   CPU_TYPE-VENDOR-KERNEL-OPERATING_SYSTEM
  std::string getDefaultTargetTriple();

  /// getProcessTriple() - Return an appropriate target triple for generating
  /// code to be loaded into the current process, e.g. when using the JIT.
  std::string getProcessTriple();

  /// getHostCPUName - Get the LLVM name for the host CPU. The particular format
  /// of the name is target dependent, and suitable for passing as -mcpu to the
  /// target which matches the host.
  ///
  /// \return - The host CPU name, or empty if the CPU could not be determined.
  StringRef getHostCPUName();

  /// getHostCPUFeatures - Get the LLVM names for the host CPU features.
  /// The particular format of the names are target dependent, and suitable for
  /// passing as -mattr to the target which matches the host.
  ///
  /// \param Features - A string mapping feature names to either
  /// true (if enabled) or false (if disabled). This routine makes no guarantees
  /// about exactly which features may appear in this map, except that they are
  /// all valid LLVM feature names.
  ///
  /// \return - True on success.
  bool getHostCPUFeatures(StringMap<bool> &Features);

  /// Get the number of physical cores (as opposed to logical cores returned
  /// from thread::hardware_concurrency(), which includes hyperthreads).
  /// Returns -1 if unknown for the current host system.
  int getHostNumPhysicalCores();

  namespace detail {
  /// Helper functions to extract HostCPUName from /proc/cpuinfo on linux.
  StringRef getHostCPUNameForPowerPC(StringRef ProcCpuinfoContent);
  StringRef getHostCPUNameForARM(StringRef ProcCpuinfoContent);
  StringRef getHostCPUNameForS390x(StringRef ProcCpuinfoContent);
  StringRef getHostCPUNameForBPF();
  }
}
}

#endif
