//===--- SymbolName.h - Clang refactoring library -------------------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_SYMBOLNAME_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_SYMBOLNAME_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
namespace tooling {

/// A name of a symbol.
///
/// Symbol's name can be composed of multiple strings. For example, Objective-C
/// methods can contain multiple argument labels:
///
/// \code
/// - (void) myMethodNamePiece: (int)x anotherNamePieces:(int)y;
/// //       ^~ string 0 ~~~~~         ^~ string 1 ~~~~~
/// \endcode
class SymbolName {
public:
  explicit SymbolName(StringRef Name) {
    // While empty symbol names are valid (Objective-C selectors can have empty
    // name pieces), occurrences Objective-C selectors are created using an
    // array of strings instead of just one string.
    assert(!Name.empty() && "Invalid symbol name!");
    this->Name.push_back(Name.str());
  }

  ArrayRef<std::string> getNamePieces() const { return Name; }

private:
  toolchain::SmallVector<std::string, 1> Name;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_SYMBOLNAME_H
