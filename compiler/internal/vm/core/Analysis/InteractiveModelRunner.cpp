//===- InteractiveModelRunner.cpp - noop ML model runner   ----------------===//
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
// A runner that communicates with an external agent via 2 file descriptors.
//===----------------------------------------------------------------------===//
#include "vm/core/Analysis/InteractiveModelRunner.h"
#include "vm/core/Analysis/MLModelRunner.h"
#include "vm/core/Analysis/TensorSpec.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

static cl::opt<bool> DebugReply(
    "interactive-model-runner-echo-reply", cl::init(false), cl::Hidden,
    cl::desc("The InteractiveModelRunner will echo back to stderr "
             "the data received from the host (for debugging purposes)."));

InteractiveModelRunner::InteractiveModelRunner(
    LLVMContext &Ctx, const std::vector<TensorSpec> &Inputs,
    const TensorSpec &Advice, StringRef OutboundName, StringRef InboundName)
    : MLModelRunner(Ctx, MLModelRunner::Kind::Interactive, Inputs.size()),
      InputSpecs(Inputs), OutputSpec(Advice),
      InEC(sys::fs::openFileForRead(InboundName, Inbound)),
      OutputBuffer(OutputSpec.getTotalTensorBufferSize()) {
  if (InEC) {
    Ctx.emitError("Cannot open inbound file: " + InEC.message());
    return;
  }
  {
    auto OutStream = std::make_unique<raw_fd_ostream>(OutboundName, OutEC);
    if (OutEC) {
      Ctx.emitError("Cannot open outbound file: " + OutEC.message());
      return;
    }
    Log = std::make_unique<Logger>(std::move(OutStream), InputSpecs, Advice,
                                   /*IncludeReward=*/false, Advice);
  }
  // Just like in the no inference case, this will allocate an appropriately
  // sized buffer.
  for (size_t I = 0; I < InputSpecs.size(); ++I)
    setUpBufferForTensor(I, InputSpecs[I], nullptr);
  Log->flush();
}

InteractiveModelRunner::~InteractiveModelRunner() {
  sys::fs::file_t FDAsOSHandle = sys::fs::convertFDToNativeFile(Inbound);
  sys::fs::closeFile(FDAsOSHandle);
}

void *InteractiveModelRunner::evaluateUntyped() {
  Log->startObservation();
  for (size_t I = 0; I < InputSpecs.size(); ++I)
    Log->logTensorValue(I, reinterpret_cast<const char *>(getTensorUntyped(I)));
  Log->endObservation();
  Log->flush();

  size_t InsPoint = 0;
  char *Buff = OutputBuffer.data();
  const size_t Limit = OutputBuffer.size();
  while (InsPoint < Limit) {
    auto ReadOrErr = ::sys::fs::readNativeFile(
        sys::fs::convertFDToNativeFile(Inbound),
        {Buff + InsPoint, OutputBuffer.size() - InsPoint});
    if (ReadOrErr.takeError()) {
      Ctx.emitError("Failed reading from inbound file");
      break;
    }
    InsPoint += *ReadOrErr;
  }
  if (DebugReply)
    dbgs() << OutputSpec.name() << ": "
           << tensorValueToString(OutputBuffer.data(), OutputSpec) << "\n";
  return OutputBuffer.data();
}
