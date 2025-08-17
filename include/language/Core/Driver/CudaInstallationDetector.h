//===-- CudaInstallationDetector.h - Cuda Instalation Detector --*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_DRIVER_CUDAINSTALLATIONDETECTOR_H
#define LANGUAGE_CORE_DRIVER_CUDAINSTALLATIONDETECTOR_H

#include "language/Core/Basic/Cuda.h"
#include "language/Core/Driver/Driver.h"
#include <bitset>

namespace language::Core {
namespace driver {

/// A class to find a viable CUDA installation
class CudaInstallationDetector {
private:
  const Driver &D;
  bool IsValid = false;
  CudaVersion Version = CudaVersion::UNKNOWN;
  std::string InstallPath;
  std::string BinPath;
  std::string LibDevicePath;
  std::string IncludePath;
  toolchain::StringMap<std::string> LibDeviceMap;

  // CUDA architectures for which we have raised an error in
  // CheckCudaVersionSupportsArch.
  mutable std::bitset<(int)OffloadArch::LAST> ArchsWithBadVersion;

public:
  CudaInstallationDetector(const Driver &D, const toolchain::Triple &HostTriple,
                           const toolchain::opt::ArgList &Args);

  void AddCudaIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const;

  /// Emit an error if Version does not support the given Arch.
  ///
  /// If either Version or Arch is unknown, does not emit an error.  Emits at
  /// most one error per Arch.
  void CheckCudaVersionSupportsArch(OffloadArch Arch) const;

  /// Check whether we detected a valid Cuda install.
  bool isValid() const { return IsValid; }
  /// Print information about the detected CUDA installation.
  void print(raw_ostream &OS) const;

  /// Get the detected Cuda install's version.
  CudaVersion version() const {
    return Version == CudaVersion::NEW ? CudaVersion::PARTIALLY_SUPPORTED
                                       : Version;
  }
  /// Get the detected Cuda installation path.
  StringRef getInstallPath() const { return InstallPath; }
  /// Get the detected path to Cuda's bin directory.
  StringRef getBinPath() const { return BinPath; }
  /// Get the detected Cuda Include path.
  StringRef getIncludePath() const { return IncludePath; }
  /// Get the detected Cuda device library path.
  StringRef getLibDevicePath() const { return LibDevicePath; }
  /// Get libdevice file for given architecture
  std::string getLibDeviceFile(StringRef Gpu) const {
    return LibDeviceMap.lookup(Gpu);
  }
  void WarnIfUnsupportedVersion() const;
};

} // namespace driver
} // namespace language::Core

#endif // LANGUAGE_CORE_DRIVER_CUDAINSTALLATIONDETECTOR_H
