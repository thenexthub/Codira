//===- lit-cpuid.cpp - Get CPU feature flags for lit exported features ----===//
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
//
// lit-cpuid obtains the feature list for the currently running CPU, and outputs
// those flags that are interesting for LLDB lit tests.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Host.h"

using namespace llvm;

int main(int argc, char **argv) {
#if defined(__i386__) || defined(_M_IX86) || \
    defined(__x86_64__) || defined(_M_X64)
  const StringMap<bool> features = sys::getHostCPUFeatures();
  if (features.empty())
    return 1;

  if (features.lookup("sse"))
    outs() << "sse\n";
  if (features.lookup("avx"))
    outs() << "avx\n";
  if (features.lookup("avx512f"))
    outs() << "avx512f\n";
#endif

  return 0;
}
