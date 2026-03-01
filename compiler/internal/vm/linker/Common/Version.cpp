//===- lib/Common/Version.cpp - LLD Version Number ---------------*- C++-=====//
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
// This file defines several version-related utility functions for LLD.
//
//===----------------------------------------------------------------------===//

#include "lld/Common/Version.h"
#include "VCSVersion.inc"
#include "llvm/Support/VCSRevision.h"

// Returns a version string, e.g.:
// LLD 14.0.0 (https://github.com/llvm/llvm-project.git
// 2d9759c7902c5cbc9a7e3ab623321d5578d51687)
std::string lld::getLLDVersion() {
#ifdef LLD_VENDOR
#define LLD_VENDOR_DISPLAY LLD_VENDOR " "
#else
#define LLD_VENDOR_DISPLAY
#endif
#if defined(LLVM_REPOSITORY) && defined(LLVM_REVISION)
  return LLD_VENDOR_DISPLAY "LLD " LLD_VERSION_STRING " (" LLVM_REPOSITORY
                            " " LLVM_REVISION ")";
#else
  return LLD_VENDOR_DISPLAY "LLD " LLD_VERSION_STRING;
#endif
#undef LLD_VENDOR_DISPLAY
}
