//===------- TargetDefines.h - Target define helpers ------------*- C++ -*-===//
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
// This file declares a series of helper functions for defining target-specific
// macros.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETDEFINES_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETDEFINES_H

#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Basic/MacroBuilder.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
namespace targets {
/// Define a macro name and standard variants.  For example if MacroName is
/// "unix", then this will define "__unix", "__unix__", and "unix" when in GNU
/// mode.
LLVM_LIBRARY_VISIBILITY
void DefineStd(language::Core::MacroBuilder &Builder, toolchain::StringRef MacroName,
               const language::Core::LangOptions &Opts);

LLVM_LIBRARY_VISIBILITY
void defineCPUMacros(language::Core::MacroBuilder &Builder, toolchain::StringRef CPUName,
                     bool Tuning = true);

LLVM_LIBRARY_VISIBILITY
void addCygMingDefines(const language::Core::LangOptions &Opts,
                       language::Core::MacroBuilder &Builder);
} // namespace targets
} // namespace language::Core
#endif // LANGUAGE_CORE_LIB_BASIC_TARGETDEFINES_H
