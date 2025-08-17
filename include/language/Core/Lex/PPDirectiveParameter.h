//===--- PPDirectiveParameter.h ---------------------------------*- C++ -*-===//
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
// This file defines the base class for preprocessor directive parameters, such
// as limit(1) or suffix(x) for #embed.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LEX_PPDIRECTIVEPARAMETER_H
#define LANGUAGE_CORE_LEX_PPDIRECTIVEPARAMETER_H

#include "language/Core/Basic/SourceLocation.h"

namespace language::Core {

/// Captures basic information about a preprocessor directive parameter.
class PPDirectiveParameter {
  SourceRange R;

public:
  PPDirectiveParameter(SourceRange R) : R(R) {}

  SourceRange getParameterRange() const { return R; }
};

} // end namespace language::Core

#endif
