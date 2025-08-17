//===- toolchain/Support/Win32/Host.inc ------------------------------*- C++ -*-===//
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
// This file implements the Win32 Host support.
//
//===----------------------------------------------------------------------===//

#include "WindowsSupport.h"
#include <cstdio>
#include <string>

using namespace toolchain;

static std::string updateTripleOSVersion(std::string Triple) {
  return Triple;
}

std::string sys::getDefaultTargetTriple() {
  const char *Triple = LLVM_DEFAULT_TARGET_TRIPLE;

  // Override the default target with an environment variable named by LLVM_TARGET_TRIPLE_ENV.
#if defined(LLVM_TARGET_TRIPLE_ENV)
  if (const char *EnvTriple = std::getenv(LLVM_TARGET_TRIPLE_ENV))
    Triple = EnvTriple;
#endif

  return Triple;
}
