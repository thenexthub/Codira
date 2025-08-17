//===--- SortJavaScriptImports.h - Sort ES6 Imports -------------*- C++ -*-===//
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
/// This file implements a sorter for JavaScript ES6 imports.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_FORMAT_SORTJAVASCRIPTIMPORTS_H
#define LANGUAGE_CORE_LIB_FORMAT_SORTJAVASCRIPTIMPORTS_H

#include "language/Core/Format/Format.h"

namespace language::Core {
namespace format {

// Sort JavaScript ES6 imports/exports in ``Code``. The generated replacements
// only monotonically increase the length of the given code.
tooling::Replacements sortJavaScriptImports(const FormatStyle &Style,
                                            StringRef Code,
                                            ArrayRef<tooling::Range> Ranges,
                                            StringRef FileName);

} // end namespace format
} // end namespace language::Core

#endif
