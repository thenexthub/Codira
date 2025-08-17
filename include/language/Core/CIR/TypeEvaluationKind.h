//===----------------------------------------------------------------------===//
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

#ifndef CLANG_CIR_TYPEEVALUATIONKIND_H
#define CLANG_CIR_TYPEEVALUATIONKIND_H

namespace cir {

// This is copied from clang/lib/CodeGen/CodeGenFunction.h.  That file (1) is
// not available as an include from ClangIR files, and (2) has lots of stuff
// that we don't want in ClangIR.
enum TypeEvaluationKind { TEK_Scalar, TEK_Complex, TEK_Aggregate };

} // namespace cir

#endif // CLANG_CIR_TYPEEVALUATIONKIND_H
