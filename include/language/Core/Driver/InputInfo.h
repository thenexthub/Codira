//===--- InputInfo.h - Input Source & Type Information ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_DRIVER_INPUTINFO_H
#define LANGUAGE_CORE_DRIVER_INPUTINFO_H

#include "language/Core/Driver/Action.h"
#include "language/Core/Driver/Types.h"
#include "toolchain/Option/Arg.h"
#include <cassert>
#include <string>

namespace language::Core {
namespace driver {

/// InputInfo - Wrapper for information about an input source.
class InputInfo {
  // FIXME: The distinction between filenames and inputarg here is
  // gross; we should probably drop the idea of a "linker
  // input". Doing so means tweaking pipelining to still create link
  // steps when it sees linker inputs (but not treat them as
  // arguments), and making sure that arguments get rendered
  // correctly.
  enum Class {
    Nothing,
    Filename,
    InputArg,
    Pipe
  };

  union {
    const char *Filename;
    const toolchain::opt::Arg *InputArg;
  } Data;
  Class Kind;
  const Action* Act;
  types::ID Type;
  const char *BaseInput;

  static types::ID GetActionType(const Action *A) {
    return A != nullptr ? A->getType() : types::TY_Nothing;
  }

public:
  InputInfo() : InputInfo(nullptr, nullptr) {}
  InputInfo(const Action *A, const char *_BaseInput)
      : Kind(Nothing), Act(A), Type(GetActionType(A)), BaseInput(_BaseInput) {}

  InputInfo(types::ID _Type, const char *_Filename, const char *_BaseInput)
      : Kind(Filename), Act(nullptr), Type(_Type), BaseInput(_BaseInput) {
    Data.Filename = _Filename;
  }
  InputInfo(const Action *A, const char *_Filename, const char *_BaseInput)
      : Kind(Filename), Act(A), Type(GetActionType(A)), BaseInput(_BaseInput) {
    Data.Filename = _Filename;
  }

  InputInfo(types::ID _Type, const toolchain::opt::Arg *_InputArg,
            const char *_BaseInput)
      : Kind(InputArg), Act(nullptr), Type(_Type), BaseInput(_BaseInput) {
    Data.InputArg = _InputArg;
  }
  InputInfo(const Action *A, const toolchain::opt::Arg *_InputArg,
            const char *_BaseInput)
      : Kind(InputArg), Act(A), Type(GetActionType(A)), BaseInput(_BaseInput) {
    Data.InputArg = _InputArg;
  }

  bool isNothing() const { return Kind == Nothing; }
  bool isFilename() const { return Kind == Filename; }
  bool isInputArg() const { return Kind == InputArg; }
  types::ID getType() const { return Type; }
  const char *getBaseInput() const { return BaseInput; }
  /// The action for which this InputInfo was created.  May be null.
  const Action *getAction() const { return Act; }
  void setAction(const Action *A) { Act = A; }

  const char *getFilename() const {
    assert(isFilename() && "Invalid accessor.");
    return Data.Filename;
  }
  const toolchain::opt::Arg &getInputArg() const {
    assert(isInputArg() && "Invalid accessor.");
    return *Data.InputArg;
  }

  /// getAsString - Return a string name for this input, for
  /// debugging.
  std::string getAsString() const {
    if (isFilename())
      return std::string("\"") + getFilename() + '"';
    else if (isInputArg())
      return "(input arg)";
    else
      return "(nothing)";
  }
};

} // end namespace driver
} // end namespace language::Core

#endif
