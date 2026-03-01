//===- CodeViewError.cpp - Error extensions for CodeView --------*- C++ -*-===//
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

#include "vm/core/DebugInfo/CodeView/CodeViewError.h"
#include "vm/core/Support/ErrorHandling.h"
#include <string>

using namespace vm::core;
using namespace vm::core::codeview;

namespace {
// FIXME: This class is only here to support the transition to toolchain::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class CodeViewErrorCategory : public std::error_category {
public:
  const char *name() const noexcept override { return "toolchain.codeview"; }
  std::string message(int Condition) const override {
    switch (static_cast<cv_error_code>(Condition)) {
    case cv_error_code::unspecified:
      return "An unknown CodeView error has occurred.";
    case cv_error_code::insufficient_buffer:
      return "The buffer is not large enough to read the requested number of "
             "bytes.";
    case cv_error_code::corrupt_record:
      return "The CodeView record is corrupted.";
    case cv_error_code::no_records:
      return "There are no records.";
    case cv_error_code::operation_unsupported:
      return "The requested operation is not supported.";
    case cv_error_code::unknown_member_record:
      return "The member record is of an unknown type.";
    }
    llvm_unreachable("Unrecognized cv_error_code");
  }
};
} // namespace

const std::error_category &toolchain::codeview::CVErrorCategory() {
  static CodeViewErrorCategory CodeViewErrCategory;
  return CodeViewErrCategory;
}

char CodeViewError::ID;
