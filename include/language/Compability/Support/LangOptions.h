//===-- language/Compability/Support/LangOptions.h ---------------------*- C++ -*-===//
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
//  This file defines the LangOptions interface, which holds the
//  configuration for LLVM's middle-end and back-end. It controls LLVM's code
//  generation into assembly or machine code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_SUPPORT_LANGOPTIONS_H_
#define LANGUAGE_COMPABILITY_SUPPORT_LANGOPTIONS_H_

#include <string>
#include <vector>

#include "toolchain/TargetParser/Triple.h"

namespace language::Compability::common {

/// Bitfields of LangOptions, split out from LangOptions to ensure
/// that this large collection of bitfields is a trivial class type.
class LangOptionsBase {

public:
  enum SignedOverflowBehaviorTy {
    // -fno-wrapv (default behavior in Flang)
    SOB_Undefined,

    // -fwrapv
    SOB_Defined,
  };

  enum FPModeKind {
    // Do not fuse FP ops
    FPM_Off,

    // Aggressively fuse FP ops (E.g. FMA).
    FPM_Fast,
  };

#define LANGOPT(Name, Bits, Default) unsigned Name : Bits;
#define ENUM_LANGOPT(Name, Type, Bits, Default)
#include "LangOptions.def"

protected:
#define LANGOPT(Name, Bits, Default)
#define ENUM_LANGOPT(Name, Type, Bits, Default) unsigned Name : Bits;
#include "LangOptions.def"
};

/// Tracks various options which control the dialect of Fortran that is
/// accepted. Based on language::Core::LangOptions
class LangOptions : public LangOptionsBase {

public:
  // Define accessors/mutators for code generation options of enumeration type.
#define LANGOPT(Name, Bits, Default)
#define ENUM_LANGOPT(Name, Type, Bits, Default) \
  Type get##Name() const { return static_cast<Type>(Name); } \
  void set##Name(Type Value) { \
    assert(static_cast<unsigned>(Value) < (1u << Bits)); \
    Name = static_cast<unsigned>(Value); \
  }
#include "LangOptions.def"

  /// Name of the IR file that contains the result of the OpenMP target
  /// host code generation.
  std::string OMPHostIRFile;

  /// List of triples passed in using -fopenmp-targets.
  std::vector<toolchain::Triple> OMPTargetTriples;

  LangOptions();
};

} // end namespace language::Compability::common

#endif // FORTRAN_SUPPORT_LANGOPTIONS_H_
