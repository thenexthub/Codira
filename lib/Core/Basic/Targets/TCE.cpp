//===--- TCE.cpp - Implement TCE target feature support -------------------===//
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
// This file implements TCE TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "TCE.h"
#include "Targets.h"
#include "language/Core/Basic/MacroBuilder.h"

using namespace language::Core;
using namespace language::Core::targets;

void TCETargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  DefineStd(Builder, "tce", Opts);
  Builder.defineMacro("__TCE__");
  Builder.defineMacro("__TCE_V1__");
}

void TCELETargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  DefineStd(Builder, "tcele", Opts);
  Builder.defineMacro("__TCE__");
  Builder.defineMacro("__TCE_V1__");
  Builder.defineMacro("__TCELE__");
  Builder.defineMacro("__TCELE_V1__");
}
