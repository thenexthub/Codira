//===-- AnalyzerHelpFlags.h - Query functions for --help flags --*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_STATICANALYZER_FRONTEND_ANALYZERHELPFLAGS_H
#define LANGUAGE_CORE_STATICANALYZER_FRONTEND_ANALYZERHELPFLAGS_H

namespace toolchain {
class raw_ostream;
} // namespace toolchain

namespace language::Core {

class CompilerInstance;

namespace ento {

void printCheckerHelp(toolchain::raw_ostream &OS, CompilerInstance &CI);
void printEnabledCheckerList(toolchain::raw_ostream &OS, CompilerInstance &CI);
void printAnalyzerConfigList(toolchain::raw_ostream &OS);
void printCheckerConfigList(toolchain::raw_ostream &OS, CompilerInstance &CI);

} // namespace ento
} // namespace language::Core

#endif
