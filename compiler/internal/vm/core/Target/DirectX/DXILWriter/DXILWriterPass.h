//===-- DXILWriterPass.h - Bitcode writing pass --------------*- C++ -*-===//
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
/// This file provides a bitcode writing pass.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_BITCODE_DXILWriterPass_H
#define LLVM_BITCODE_DXILWriterPass_H

#include "DirectX.h"
#include "vm/core/Bitcode/BitcodeWriter.h"
#include "vm/core/IR/PassManager.h"

namespace vm::core {
class Module;
class raw_ostream;

/// Create and return a pass that writes the module to the specified
/// ostream. Note that this pass is designed for use with the legacy pass
/// manager.
ModulePass *createDXILWriterPass(raw_ostream &Str);

/// Create and return a pass that writes the module to a global variable in the
/// module for later emission in the MCStreamer. Note that this pass is designed
/// for use with the legacy pass manager because it is run in CodeGen only.
ModulePass *createDXILEmbedderPass();

} // namespace vm::core

#endif
