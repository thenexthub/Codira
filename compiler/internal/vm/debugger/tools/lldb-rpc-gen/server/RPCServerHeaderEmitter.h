//===-- RPCServerHeaderEmitter.h ----------------------------------------===//
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

#ifndef LLDB_RPC_GEN_RPCSERVERHEADEREMITTER_H
#define LLDB_RPC_GEN_RPCSERVERHEADEREMITTER_H

#include "RPCCommon.h"

#include "clang/AST/AST.h"
#include "llvm/Support/ToolOutputFile.h"

using namespace clang;

namespace lldb_rpc_gen {
/// Emit the source code for server-side *.h files.
class RPCServerHeaderEmitter : public FileEmitter {
public:
  RPCServerHeaderEmitter(std::unique_ptr<llvm::ToolOutputFile> &&OutputFile)
      : FileEmitter(std::move(OutputFile)) {
    Begin();
  }

  ~RPCServerHeaderEmitter() { End(); }

  void EmitMethod(const Method &method);

private:
  void EmitHandleRPCCall();

  void EmitConstructor(const std::string &MangledName);

  void EmitDestructor(const std::string &MangledName);

  std::string GetHeaderGuard();

  void Begin();

  void End();
};
} // namespace lldb_rpc_gen

#endif // LLDB_RPC_GEN_RPCSERVERHEADEREMITTER_H
