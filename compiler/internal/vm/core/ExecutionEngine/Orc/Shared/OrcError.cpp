//===---------------- OrcError.cpp - Error codes for ORC ------------------===//
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
// Error codes for ORC.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/Orc/Shared/OrcError.h"
#include "vm/core/Support/ErrorHandling.h"

#include <type_traits>

using namespace vm::core;
using namespace vm::core::orc;

namespace {

// FIXME: This class is only here to support the transition to toolchain::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class OrcErrorCategory : public std::error_category {
public:
  const char *name() const noexcept override { return "orc"; }

  std::string message(int condition) const override {
    switch (static_cast<OrcErrorCode>(condition)) {
    case OrcErrorCode::UnknownORCError:
      return "Unknown ORC error";
    case OrcErrorCode::DuplicateDefinition:
      return "Duplicate symbol definition";
    case OrcErrorCode::JITSymbolNotFound:
      return "JIT symbol not found";
    case OrcErrorCode::RemoteAllocatorDoesNotExist:
      return "Remote allocator does not exist";
    case OrcErrorCode::RemoteAllocatorIdAlreadyInUse:
      return "Remote allocator Id already in use";
    case OrcErrorCode::RemoteMProtectAddrUnrecognized:
      return "Remote mprotect call references unallocated memory";
    case OrcErrorCode::RemoteIndirectStubsOwnerDoesNotExist:
      return "Remote indirect stubs owner does not exist";
    case OrcErrorCode::RemoteIndirectStubsOwnerIdAlreadyInUse:
      return "Remote indirect stubs owner Id already in use";
    case OrcErrorCode::RPCConnectionClosed:
      return "RPC connection closed";
    case OrcErrorCode::RPCCouldNotNegotiateFunction:
      return "Could not negotiate RPC function";
    case OrcErrorCode::RPCResponseAbandoned:
      return "RPC response abandoned";
    case OrcErrorCode::UnexpectedRPCCall:
      return "Unexpected RPC call";
    case OrcErrorCode::UnexpectedRPCResponse:
      return "Unexpected RPC response";
    case OrcErrorCode::UnknownErrorCodeFromRemote:
      return "Unknown error returned from remote RPC function "
             "(Use StringError to get error message)";
    case OrcErrorCode::UnknownResourceHandle:
      return "Unknown resource handle";
    case OrcErrorCode::MissingSymbolDefinitions:
      return "MissingSymbolsDefinitions";
    case OrcErrorCode::UnexpectedSymbolDefinitions:
      return "UnexpectedSymbolDefinitions";
    }
    llvm_unreachable("Unhandled error code");
  }
};

OrcErrorCategory &getOrcErrCat() {
  static OrcErrorCategory OrcErrCat;
  return OrcErrCat;
}
} // namespace

namespace vm::core {
namespace orc {

char DuplicateDefinition::ID = 0;
char JITSymbolNotFound::ID = 0;

std::error_code orcError(OrcErrorCode ErrCode) {
  typedef std::underlying_type_t<OrcErrorCode> UT;
  return std::error_code(static_cast<UT>(ErrCode), getOrcErrCat());
}

DuplicateDefinition::DuplicateDefinition(std::string SymbolName,
                                         std::optional<std::string> Context)
    : SymbolName(std::move(SymbolName)), Context(std::move(Context)) {}

std::error_code DuplicateDefinition::convertToErrorCode() const {
  return orcError(OrcErrorCode::DuplicateDefinition);
}

void DuplicateDefinition::log(raw_ostream &OS) const {
  if (Context)
    OS << "In " << *Context << ", ";
  OS << "duplicate definition of symbol '" << SymbolName << "'";
}

const std::string &DuplicateDefinition::getSymbolName() const {
  return SymbolName;
}

const std::optional<std::string> &DuplicateDefinition::getContext() const {
  return Context;
}

JITSymbolNotFound::JITSymbolNotFound(std::string SymbolName)
    : SymbolName(std::move(SymbolName)) {}

std::error_code JITSymbolNotFound::convertToErrorCode() const {
  typedef std::underlying_type_t<OrcErrorCode> UT;
  return std::error_code(static_cast<UT>(OrcErrorCode::JITSymbolNotFound),
                         getOrcErrCat());
}

void JITSymbolNotFound::log(raw_ostream &OS) const {
  OS << "Could not find symbol '" << SymbolName << "'";
}

const std::string &JITSymbolNotFound::getSymbolName() const {
  return SymbolName;
}

} // namespace orc
} // namespace vm::core
