//===- OptionUtils.h - Utilities for command line arguments -----*- C++ -*-===//
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
//  This header contains utilities for command line arguments.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_DRIVER_OPTIONUTILS_H
#define LANGUAGE_CORE_DRIVER_OPTIONUTILS_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/Option/OptSpecifier.h"

namespace toolchain {

namespace opt {

class ArgList;

} // namespace opt

} // namespace toolchain

namespace language::Core {
/// Return the value of the last argument as an integer, or a default. If Diags
/// is non-null, emits an error if the argument is given, but non-integral.
int getLastArgIntValue(const toolchain::opt::ArgList &Args,
                       toolchain::opt::OptSpecifier Id, int Default,
                       DiagnosticsEngine *Diags = nullptr, unsigned Base = 0);

inline int getLastArgIntValue(const toolchain::opt::ArgList &Args,
                              toolchain::opt::OptSpecifier Id, int Default,
                              DiagnosticsEngine &Diags, unsigned Base = 0) {
  return getLastArgIntValue(Args, Id, Default, &Diags, Base);
}

uint64_t getLastArgUInt64Value(const toolchain::opt::ArgList &Args,
                               toolchain::opt::OptSpecifier Id, uint64_t Default,
                               DiagnosticsEngine *Diags = nullptr,
                               unsigned Base = 0);

inline uint64_t getLastArgUInt64Value(const toolchain::opt::ArgList &Args,
                                      toolchain::opt::OptSpecifier Id,
                                      uint64_t Default,
                                      DiagnosticsEngine &Diags,
                                      unsigned Base = 0) {
  return getLastArgUInt64Value(Args, Id, Default, &Diags, Base);
}

} // namespace language::Core

#endif // LANGUAGE_CORE_DRIVER_OPTIONUTILS_H
