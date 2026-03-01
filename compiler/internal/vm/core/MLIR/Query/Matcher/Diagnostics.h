//===--- Diagnostics.h - Helper class for error diagnostics -----*- C++ -*-===//
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
//
// Diagnostics class to manage error messages. Implementation shares similarity
// to clang-query Diagnostics.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_TOOLS_MLIRQUERY_MATCHER_DIAGNOSTICS_H
#define MLIR_TOOLS_MLIRQUERY_MATCHER_DIAGNOSTICS_H

#include "mlir/Query/Matcher/ErrorBuilder.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/raw_ostream.h"
#include <string>
#include <vector>

namespace mlir::query::matcher::internal {

// Diagnostics class to manage error messages.
class Diagnostics {
public:
  // Helper stream class for constructing error messages.
  class ArgStream {
  public:
    ArgStream(std::vector<std::string> *out) : out(out) {}
    template <class T>
    ArgStream &operator<<(const T &arg) {
      return operator<<(toolchain::Twine(arg));
    }
    ArgStream &operator<<(const toolchain::Twine &arg);

  private:
    std::vector<std::string> *out;
  };

  // Add an error message with the specified range and error type.
  // Returns an ArgStream object to allow constructing the error message using
  // the << operator.
  ArgStream addError(SourceRange range, ErrorType error);

  // Print all error messages to the specified output stream.
  void print(toolchain::raw_ostream &os) const;

private:
  // Information stored for one frame of the context.
  struct ContextFrame {
    SourceRange range;
    std::vector<std::string> args;
  };

  // Information stored for each error found.
  struct ErrorContent {
    std::vector<ContextFrame> contextStack;
    struct Message {
      SourceRange range;
      ErrorType type;
      std::vector<std::string> args;
    };
    std::vector<Message> messages;
  };

  void printMessage(const ErrorContent::Message &message,
                    const toolchain::Twine Prefix, toolchain::raw_ostream &os) const;

  void printErrorContent(const ErrorContent &content,
                         toolchain::raw_ostream &os) const;

  std::vector<ContextFrame> contextStack;
  std::vector<ErrorContent> errorValues;
};

} // namespace mlir::query::matcher::internal

#endif // MLIR_TOOLS_MLIRQUERY_MATCHER_DIAGNOSTICS_H
