//===--- TargetOptions.h ----------------------------------------*- C++ -*-===//
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
///
/// \file
/// Defines the flang::TargetOptions class.
///
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_FRONTEND_TARGETOPTIONS_H
#define LANGUAGE_COMPABILITY_FRONTEND_TARGETOPTIONS_H

#include <string>
#include <vector>

namespace language::Compability::frontend {

/// Options for controlling the target.
class TargetOptions {
public:
  /// The name of the target triple to compile for.
  std::string triple;

  /// If given, the name of the target CPU to generate code for.
  std::string cpu;

  /// If given, the name of the target CPU to tune code for.
  std::string cpuToTuneFor;

  /// If given, the name of the target ABI to use.
  std::string abi;

  /// The list of target specific features to enable or disable, as written on
  /// the command line.
  std::vector<std::string> featuresAsWritten;

  /// The real KINDs disabled for this target
  std::vector<int> disabledRealKinds;

  /// The integer KINDs disabled for this target
  std::vector<int> disabledIntegerKinds;

  /// Extended Altivec ABI on AIX
  bool EnableAIXExtendedAltivecABI;

  /// Print verbose assembly
  bool asmVerbose = false;

  /// Atomic control options
  bool atomicIgnoreDenormalMode = false;
  bool atomicRemoteMemory = false;
  bool atomicFineGrainedMemory = false;
};

} // end namespace language::Compability::frontend

#endif
