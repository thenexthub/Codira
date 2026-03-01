//===- PassInstrumentation.cpp - Pass Instrumentation interface -*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
/// \file
///
/// This file provides the implementation of PassInstrumentation class.
///
//===----------------------------------------------------------------------===//

#include "vm/core/IR/PassInstrumentation.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/IR/PassManager.h"

using namespace vm::core;

template struct LLVM_EXPORT_TEMPLATE Any::TypeId<const Module *>;
template struct LLVM_EXPORT_TEMPLATE Any::TypeId<const Function *>;
template struct LLVM_EXPORT_TEMPLATE Any::TypeId<const Loop *>;

void PassInstrumentationCallbacks::addClassToPassName(StringRef ClassName,
                                                      StringRef PassName) {
  assert(!PassName.empty() && "PassName can't be empty!");
  ClassToPassName.try_emplace(ClassName, PassName.str());
}

StringRef
PassInstrumentationCallbacks::getPassNameForClassName(StringRef ClassName) {
  if (!ClassToPassNameCallbacks.empty()) {
    for (auto &Fn : ClassToPassNameCallbacks)
      Fn();
    ClassToPassNameCallbacks.clear();
  }
  auto PassNameIter = ClassToPassName.find(ClassName);
  if (PassNameIter != ClassToPassName.end())
    return PassNameIter->second;
  return {};
}

AnalysisKey PassInstrumentationAnalysis::Key;

bool toolchain::isSpecialPass(StringRef PassID,
                         const std::vector<StringRef> &Specials) {
  size_t Pos = PassID.find('<');
  StringRef Prefix = PassID;
  if (Pos != StringRef::npos)
    Prefix = PassID.substr(0, Pos);
  return any_of(Specials,
                [Prefix](StringRef S) { return Prefix.ends_with(S); });
}
