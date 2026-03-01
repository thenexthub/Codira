//===-- RPCServerHeaderEmitter.cpp ----------------------------------------===//
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

#include "RPCServerHeaderEmitter.h"
#include "RPCCommon.h"

#include "clang/AST/AST.h"
#include "clang/AST/Mangle.h"
#include "clang/Frontend/CompilerInstance.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace lldb_rpc_gen;

void RPCServerHeaderEmitter::EmitMethod(const Method &method) {
  // We'll be using the mangled name in order to disambiguate
  // overloaded methods.
  const std::string &MangledName = method.MangledName;

  EmitLine("class " + MangledName +
           " : public rpc_common::RPCFunctionInstance {");
  EmitLine("public:");
  IndentLevel++;
  EmitConstructor(MangledName);
  EmitDestructor(MangledName);
  EmitHandleRPCCall();
  IndentLevel--;
  EmitLine("};");
}

void RPCServerHeaderEmitter::EmitHandleRPCCall() {
  EmitLine("bool HandleRPCCall(rpc_common::Connection &connection, "
           "rpc_common::RPCStream &send, rpc_common::RPCStream &response) "
           "override;");
}

void RPCServerHeaderEmitter::EmitConstructor(const std::string &MangledName) {
  EmitLine(MangledName + "() : RPCFunctionInstance(\"" + MangledName +
           "\") {}");
}

void RPCServerHeaderEmitter::EmitDestructor(const std::string &MangledName) {
  EmitLine("~" + MangledName + "() override {}");
}

std::string RPCServerHeaderEmitter::GetHeaderGuard() {
  const std::string UpperFilenameNoExt =
      llvm::sys::path::stem(
          llvm::sys::path::filename(OutputFile->getFilename()))
          .upper();
  return "GENERATED_LLDB_RPC_SERVER_" + UpperFilenameNoExt + "_H";
}

void RPCServerHeaderEmitter::Begin() {
  const std::string HeaderGuard = GetHeaderGuard();
  EmitLine("#ifndef " + HeaderGuard);
  EmitLine("#define " + HeaderGuard);
  EmitLine("");
  EmitLine("#include <lldb-rpc/common/RPCFunction.h>");
  EmitLine("");
  EmitLine("namespace rpc_server {");
}

void RPCServerHeaderEmitter::End() {
  EmitLine("} // namespace rpc_server");
  EmitLine("#endif // " + GetHeaderGuard());
}
