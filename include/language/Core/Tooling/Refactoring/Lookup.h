//===--- Lookup.h - Framework for clang refactoring tools --*- C++ -*------===//
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
//  This file defines helper methods for clang tools performing name lookup.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_LOOKUP_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_LOOKUP_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include <string>

namespace language::Core {

class DeclContext;
class NamedDecl;
class NestedNameSpecifier;

namespace tooling {

/// Emulate a lookup to replace one nested name specifier with another using as
/// few additional namespace qualifications as possible.
///
/// This does not perform a full C++ lookup so ADL will not work.
///
/// \param Use The nested name to be replaced.
/// \param UseLoc The location of name to be replaced.
/// \param UseContext The context in which the nested name is contained. This
///                   will be used to minimize namespace qualifications.
/// \param FromDecl The declaration to which the nested name points.
/// \param ReplacementString The replacement nested name. Must be fully
///                          qualified including a leading "::".
/// \returns The new name to be inserted in place of the current nested name.
std::string replaceNestedName(NestedNameSpecifier Use, SourceLocation UseLoc,
                              const DeclContext *UseContext,
                              const NamedDecl *FromDecl,
                              StringRef ReplacementString);

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_LOOKUP_H
