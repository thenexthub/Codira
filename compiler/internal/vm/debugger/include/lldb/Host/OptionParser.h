//===-- OptionParser.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_OPTIONPARSER_H
#define LLDB_HOST_OPTIONPARSER_H

#include <mutex>
#include <string>

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ArrayRef.h"

struct option;

namespace lldb_private {

struct OptionDefinition;

struct Option {
  // The definition of the option that this refers to.
  const OptionDefinition *definition;
  // if not NULL, set *flag to val when option found
  int *flag;
  // if flag not NULL, value to set *flag to; else return value
  int val;
};

class OptionParser {
public:
  enum OptionArgument { eNoArgument = 0, eRequiredArgument, eOptionalArgument };

  static void Prepare(std::unique_lock<std::mutex> &lock);

  static void EnableError(bool error);

  /// Argv must be an argument vector "as passed to main", i.e. terminated with
  /// a nullptr.
  static int Parse(llvm::MutableArrayRef<char *> argv,
                   llvm::StringRef optstring, const Option *longopts,
                   int *longindex);

  static char *GetOptionArgument();
  static int GetOptionIndex();
  static int GetOptionErrorCause();
  static std::string GetShortOptionString(struct option *long_options);
};
}

#endif // LLDB_HOST_OPTIONPARSER_H
