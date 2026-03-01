//===- Error.cpp - system_error extensions for PDB --------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/DebugInfo/PDB/GenericError.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;
using namespace vm::core::pdb;

namespace {
// FIXME: This class is only here to support the transition to toolchain::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class PDBErrorCategory : public std::error_category {
public:
  const char *name() const noexcept override { return "toolchain.pdb"; }
  std::string message(int Condition) const override {
    switch (static_cast<pdb_error_code>(Condition)) {
    case pdb_error_code::unspecified:
      return "An unknown error has occurred.";
    case pdb_error_code::dia_sdk_not_present:
      return "LLVM was not compiled with support for DIA. This usually means "
             "that you are not using MSVC, or your Visual Studio "
             "installation is corrupt.";
    case pdb_error_code::dia_failed_loading:
      return "DIA is only supported when using MSVC.";
    case pdb_error_code::invalid_utf8_path:
      return "The PDB file path is an invalid UTF8 sequence.";
    case pdb_error_code::signature_out_of_date:
      return "The signature does not match; the file(s) might be out of date.";
    case pdb_error_code::no_matching_pch:
      return "No matching precompiled header could be located.";
    }
    llvm_unreachable("Unrecognized generic_error_code");
  }
};
} // namespace

const std::error_category &toolchain::pdb::PDBErrCategory() {
  static PDBErrorCategory PDBCategory;
  return PDBCategory;
}

char PDBError::ID;
