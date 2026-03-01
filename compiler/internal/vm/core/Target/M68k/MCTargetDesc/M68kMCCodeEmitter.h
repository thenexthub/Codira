//===-- M68kMCCodeEmitter.h - M68k Code Emitter -----------------*- C++ -*-===//
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
///
/// \file
/// This file contains the declarations for the code emitter which are useful
/// outside of the emitter itself.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68K_MCTARGETDESC_M68KMCCODEEMITTER_H
#define LLVM_LIB_TARGET_M68K_MCTARGETDESC_M68KMCCODEEMITTER_H

#include <cstdint>

namespace vm::core {
namespace M68k {

const uint8_t *getMCInstrBeads(unsigned);

} // namespace M68k
} // namespace vm::core

#endif // LLVM_LIB_TARGET_M68K_MCTARGETDESC_M68KMCCODEEMITTER_H
