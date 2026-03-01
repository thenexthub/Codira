//===-- SourceRequestHandler.cpp ------------------------------------------===//
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

#include "DAP.h"
#include "Handler/RequestHandler.h"
#include "LLDBUtils.h"
#include "Protocol/ProtocolRequests.h"
#include "Protocol/ProtocolTypes.h"
#include "lldb/API/SBAddress.h"
#include "lldb/API/SBExecutionContext.h"
#include "lldb/API/SBFrame.h"
#include "lldb/API/SBInstructionList.h"
#include "lldb/API/SBProcess.h"
#include "lldb/API/SBStream.h"
#include "lldb/API/SBSymbol.h"
#include "lldb/API/SBTarget.h"
#include "lldb/API/SBThread.h"
#include "lldb/lldb-types.h"
#include "llvm/Support/Error.h"

namespace lldb_dap {

/// Source request; value of command field is 'source'. The request retrieves
/// the source code for a given source reference.
llvm::Expected<protocol::SourceResponseBody>
SourceRequestHandler::Run(const protocol::SourceArguments &args) const {

  uint32_t source_ref =
      args.source->sourceReference.value_or(args.sourceReference);
  const std::optional<lldb::addr_t> source_addr_opt =
      dap.GetSourceReferenceAddress(source_ref);

  if (!source_addr_opt)
    return llvm::make_error<DAPError>(
        llvm::formatv("Unknown source reference {}", source_ref));

  lldb::SBAddress address(*source_addr_opt, dap.target);
  if (!address.IsValid())
    return llvm::make_error<DAPError>("source not found");

  lldb::SBSymbol symbol = address.GetSymbol();
  lldb::SBInstructionList insts;

  if (symbol.IsValid()) {
    insts = symbol.GetInstructions(dap.target);
  } else {
    // No valid symbol, just return the disassembly.
    insts = dap.target.ReadInstructions(
        address, dap.k_number_of_assembly_lines_for_nodebug);
  }

  if (!insts || insts.GetSize() == 0)
    return llvm::make_error<DAPError>(
        llvm::formatv("no instruction source for address {}",
                      address.GetLoadAddress(dap.target)));

  lldb::SBStream stream;
  lldb::SBExecutionContext exe_ctx(dap.target);
  insts.GetDescription(stream, exe_ctx);
  return protocol::SourceResponseBody{/*content=*/stream.GetData(),
                                      /*mimeType=*/
                                      "text/x-lldb.disassembly"};
}

} // namespace lldb_dap
