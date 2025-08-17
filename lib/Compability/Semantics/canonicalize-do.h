//===-- lib/Semantics/canonicalize-do.h -------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_CANONICALIZE_DO_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_CANONICALIZE_DO_H_

// Converts a LabelDo followed by a sequence of ExecutableConstructs (perhaps
// logically nested) into the more structured DoConstruct (explicitly nested)
namespace language::Compability::parser {
struct Program;
bool CanonicalizeDo(Program &program);
} // namespace language::Compability::parser

#endif // FORTRAN_SEMANTICS_CANONICALIZE_DO_H_
