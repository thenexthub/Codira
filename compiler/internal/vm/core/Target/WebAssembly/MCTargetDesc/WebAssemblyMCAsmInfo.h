//===-- WebAssemblyMCAsmInfo.h - WebAssembly asm properties -----*- C++ -*-===//
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
/// This file contains the declaration of the WebAssemblyMCAsmInfo class.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_MCTARGETDESC_WEBASSEMBLYMCASMINFO_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_MCTARGETDESC_WEBASSEMBLYMCASMINFO_H

#include "vm/core/MC/MCAsmInfoWasm.h"

namespace vm::core {

class Triple;

class WebAssemblyMCAsmInfo final : public MCAsmInfoWasm {
public:
  explicit WebAssemblyMCAsmInfo(const Triple &T,
                                const MCTargetOptions &Options);
  ~WebAssemblyMCAsmInfo() override;
};

namespace WebAssembly {
enum Specifier {
  S_None,
  S_FUNCINDEX, // Wasm function index
  S_GOT,
  S_GOT_TLS,   // Wasm global index of TLS symbol
  S_MBREL,     // Memory address relative to __memory_base
  S_TBREL,     // Table index relative to __table_base
  S_TLSREL,    // Memory address relative to __tls_base
  S_TYPEINDEX, // Reference to a symbol's type (signature)
};
}
} // end namespace vm::core

#endif
