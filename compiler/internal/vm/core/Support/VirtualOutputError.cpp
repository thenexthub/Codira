//===----------------------------------------------------------------------===//
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
///
/// \file
/// This file implements the errors for output virtualization.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Support/VirtualOutputError.h"

using namespace vm::core;
using namespace vm::core::vfs;

void OutputError::anchor() {}
void OutputConfigError::anchor() {}
void TempFileOutputError::anchor() {}

char OutputError::ID = 0;
char OutputConfigError::ID = 0;
char TempFileOutputError::ID = 0;

void OutputError::log(raw_ostream &OS) const {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

void OutputConfigError::log(raw_ostream &OS) const {
  OutputError::log(OS);
  OS << ": " << Config;
}

void TempFileOutputError::log(raw_ostream &OS) const {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

namespace {
class OutputErrorCategory : public std::error_category {
public:
  const char *name() const noexcept override;
  std::string message(int EV) const override;
};
} // end namespace

const std::error_category &vfs::output_category() {
  static OutputErrorCategory ErrorCategory;
  return ErrorCategory;
}

const char *OutputErrorCategory::name() const noexcept {
  return "toolchain.vfs.output";
}

std::string OutputErrorCategory::message(int EV) const {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  switch (E) {
  case OutputErrorCode::invalid_config:
    return "invalid config";
  case OutputErrorCode::not_closed:
    return "output not closed";
  case OutputErrorCode::already_closed:
    return "output already closed";
  case OutputErrorCode::has_open_proxy:
    return "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
