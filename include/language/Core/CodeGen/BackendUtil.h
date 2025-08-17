//===--- BackendUtil.h - LLVM Backend Utilities -----------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_CODEGEN_BACKENDUTIL_H
#define LANGUAGE_CORE_CODEGEN_BACKENDUTIL_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/IR/ModuleSummaryIndex.h"
#include <memory>

namespace toolchain {
class BitcodeModule;
template <typename T> class Expected;
template <typename T> class IntrusiveRefCntPtr;
class Module;
class MemoryBufferRef;
namespace vfs {
class FileSystem;
} // namespace vfs
} // namespace toolchain

namespace language::Core {
class CompilerInstance;
class DiagnosticsEngine;
class CodeGenOptions;
class BackendConsumer;

enum BackendAction {
  Backend_EmitAssembly, ///< Emit native assembly files
  Backend_EmitBC,       ///< Emit LLVM bitcode files
  Backend_EmitLL,       ///< Emit human-readable LLVM assembly
  Backend_EmitNothing,  ///< Don't emit anything (benchmarking mode)
  Backend_EmitMCNull,   ///< Run CodeGen, but don't emit anything
  Backend_EmitObj       ///< Emit native object files
};

void emitBackendOutput(CompilerInstance &CI, CodeGenOptions &CGOpts,
                       StringRef TDesc, toolchain::Module *M, BackendAction Action,
                       toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS,
                       std::unique_ptr<raw_pwrite_stream> OS,
                       BackendConsumer *BC = nullptr);

void EmbedBitcode(toolchain::Module *M, const CodeGenOptions &CGOpts,
                  toolchain::MemoryBufferRef Buf);

void EmbedObject(toolchain::Module *M, const CodeGenOptions &CGOpts,
                 DiagnosticsEngine &Diags);
} // namespace language::Core

#endif
