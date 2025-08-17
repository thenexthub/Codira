//===- TemplateArgumentHasher.h - Hash Template Arguments -------*- C++ -*-===//
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

#include "language/Core/AST/TemplateBase.h"

namespace language::Core {
namespace serialization {

/// Calculate a stable hash value for template arguments. We guarantee that
/// the same template arguments must have the same hashed values. But we don't
/// guarantee that the template arguments with the same hashed value are the
/// same template arguments.
///
/// ODR hashing may not be the best mechanism to hash the template
/// arguments. ODR hashing is (or perhaps, should be) about determining whether
/// two things are spelled the same way and have the same meaning (as required
/// by the C++ ODR), whereas what we want here is whether they have the same
/// meaning regardless of spelling. Maybe we can get away with reusing ODR
/// hashing anyway, on the basis that any canonical, non-dependent template
/// argument should have the same (invented) spelling in every translation
/// unit, but it is not sure that's true in all cases. There may still be cases
/// where the canonical type includes some aspect of "whatever we saw first",
/// in which case the ODR hash can differ across translation units for
/// non-dependent, canonical template arguments that are spelled differently
/// but have the same meaning. But it is not easy to raise examples.
unsigned StableHashForTemplateArguments(toolchain::ArrayRef<TemplateArgument> Args);

} // namespace serialization
} // namespace language::Core
