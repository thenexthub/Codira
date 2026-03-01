// CodeGen/RuntimeLibcallSignatures.h - R.T. Lib. Call Signatures -*- C++ -*--//
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
/// This file provides signature information for runtime libcalls.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_RUNTIME_LIBCALL_SIGNATURES_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_RUNTIME_LIBCALL_SIGNATURES_H

#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/RuntimeLibcallUtil.h"

namespace vm::core {

class WebAssemblySubtarget;

namespace WebAssembly {

void getLibcallSignature(const WebAssemblySubtarget &Subtarget,
                         RTLIB::Libcall LC,
                         SmallVectorImpl<wasm::ValType> &Rets,
                         SmallVectorImpl<wasm::ValType> &Params);

void getLibcallSignature(const WebAssemblySubtarget &Subtarget, StringRef Name,
                         SmallVectorImpl<wasm::ValType> &Rets,
                         SmallVectorImpl<wasm::ValType> &Params);

} // end namespace WebAssembly
} // end namespace vm::core

#endif
