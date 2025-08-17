//===--- MacroBuilder.h - CPP Macro building utility ------------*- C++ -*-===//
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
/// Defines the language::Core::MacroBuilder utility class.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_MACROBUILDER_H
#define LANGUAGE_CORE_BASIC_MACROBUILDER_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {

class MacroBuilder {
  raw_ostream &Out;
public:
  MacroBuilder(raw_ostream &Output) : Out(Output) {}

  /// Append a \#define line for macro of the form "\#define Name Value\n".
  /// If DeprecationMsg is provided, also append a pragma to deprecate the
  /// defined macro.
  void defineMacro(const Twine &Name, const Twine &Value = "1",
                   Twine DeprecationMsg = "") {
    Out << "#define " << Name << ' ' << Value << '\n';
    if (!DeprecationMsg.isTriviallyEmpty())
      Out << "#pragma clang deprecated(" << Name << ", \"" << DeprecationMsg
          << "\")\n";
  }

  /// Append a \#undef line for Name.  Name should be of the form XXX
  /// and we emit "\#undef XXX".
  void undefineMacro(const Twine &Name) {
    Out << "#undef " << Name << '\n';
  }

  /// Directly append Str and a newline to the underlying buffer.
  void append(const Twine &Str) {
    Out << Str << '\n';
  }
};

}  // end namespace language::Core

#endif
