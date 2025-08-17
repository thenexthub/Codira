//===--- OptionUtils.cpp - Utilities for command line arguments -----------===//
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

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/DiagnosticDriver.h"
#include "language/Core/Driver/OptionUtils.h"
#include "toolchain/Option/ArgList.h"

using namespace language::Core;
using namespace toolchain::opt;

namespace {
template <typename IntTy>
IntTy getLastArgIntValueImpl(const ArgList &Args, OptSpecifier Id,
                             IntTy Default, DiagnosticsEngine *Diags,
                             unsigned Base) {
  IntTy Res = Default;
  if (Arg *A = Args.getLastArg(Id)) {
    if (StringRef(A->getValue()).getAsInteger(Base, Res)) {
      if (Diags)
        Diags->Report(diag::err_drv_invalid_int_value)
            << A->getAsString(Args) << A->getValue();
    }
  }
  return Res;
}
} // namespace

namespace language::Core {

int getLastArgIntValue(const ArgList &Args, OptSpecifier Id, int Default,
                       DiagnosticsEngine *Diags, unsigned Base) {
  return getLastArgIntValueImpl<int>(Args, Id, Default, Diags, Base);
}

uint64_t getLastArgUInt64Value(const ArgList &Args, OptSpecifier Id,
                               uint64_t Default, DiagnosticsEngine *Diags,
                               unsigned Base) {
  return getLastArgIntValueImpl<uint64_t>(Args, Id, Default, Diags, Base);
}

} // namespace language::Core
