//===-- Valgrind.cpp - Implement Valgrind communication ---------*- C++ -*-===//
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
//  Defines Valgrind communication methods, if HAVE_VALGRIND_VALGRIND_H is
//  defined.  If we have valgrind.h but valgrind isn't running, its macros are
//  no-ops.
//
//===----------------------------------------------------------------------===//

#include <IndexStoreDB_LLVMSupport/toolchain_Support_Valgrind.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Config_config.h>
#include <cstddef>

#if HAVE_VALGRIND_VALGRIND_H
#include <valgrind_valgrind.h>

static bool InitNotUnderValgrind() {
  return !RUNNING_ON_VALGRIND;
}

// This bool is negated from what we'd expect because code may run before it
// gets initialized.  If that happens, it will appear to be 0 (false), and we
// want that to cause the rest of the code in this file to run the
// Valgrind-provided macros.
static const bool NotUnderValgrind = InitNotUnderValgrind();

bool toolchain::sys::RunningOnValgrind() {
  if (NotUnderValgrind)
    return false;
  return RUNNING_ON_VALGRIND;
}

void toolchain::sys::ValgrindDiscardTranslations(const void *Addr, size_t Len) {
  if (NotUnderValgrind)
    return;

  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#else  // !HAVE_VALGRIND_VALGRIND_H

bool toolchain::sys::RunningOnValgrind() {
  return false;
}

void toolchain::sys::ValgrindDiscardTranslations(const void *Addr, size_t Len) {
}

#endif  // !HAVE_VALGRIND_VALGRIND_H
