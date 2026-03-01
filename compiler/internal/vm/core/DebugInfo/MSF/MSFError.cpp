//===- MSFError.cpp - Error extensions for MSF files ------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/MSF/MSFError.h"
#include "vm/core/Support/ErrorHandling.h"
#include <string>

using namespace vm::core;
using namespace vm::core::msf;

namespace {
// FIXME: This class is only here to support the transition to toolchain::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class MSFErrorCategory : public std::error_category {
public:
  const char *name() const noexcept override { return "toolchain.msf"; }
  std::string message(int Condition) const override {
    switch (static_cast<msf_error_code>(Condition)) {
    case msf_error_code::unspecified:
      return "An unknown error has occurred.";
    case msf_error_code::insufficient_buffer:
      return "The buffer is not large enough to read the requested number of "
             "bytes.";
    case msf_error_code::size_overflow_4096:
      return "Output data is larger than 4 GiB.";
    case msf_error_code::size_overflow_8192:
      return "Output data is larger than 8 GiB.";
    case msf_error_code::size_overflow_16384:
      return "Output data is larger than 16 GiB.";
    case msf_error_code::size_overflow_32768:
      return "Output data is larger than 32 GiB.";
    case msf_error_code::not_writable:
      return "The specified stream is not writable.";
    case msf_error_code::no_stream:
      return "The specified stream does not exist.";
    case msf_error_code::invalid_format:
      return "The data is in an unexpected format.";
    case msf_error_code::block_in_use:
      return "The block is already in use.";
    case msf_error_code::stream_directory_overflow:
      return "PDB stream directory too large.";
    }
    llvm_unreachable("Unrecognized msf_error_code");
  }
};
} // namespace

const std::error_category &toolchain::msf::MSFErrCategory() {
  static MSFErrorCategory MSFCategory;
  return MSFCategory;
}

char MSFError::ID;
