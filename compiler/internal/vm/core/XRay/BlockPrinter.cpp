//===- BlockPrinter.cpp - FDR Block Pretty Printer Implementation --------===//
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
#include "vm/core/XRay/BlockPrinter.h"

using namespace vm::core;
using namespace vm::core::xray;

Error BlockPrinter::visit(BufferExtents &R) {
  OS << "\n[New Block]\n";
  CurrentState = State::Preamble;
  return RP.visit(R);
}

// Preamble printing.
Error BlockPrinter::visit(NewBufferRecord &R) {
  if (CurrentState == State::Start)
    OS << "\n[New Block]\n";

  OS << "Preamble: \n";
  CurrentState = State::Preamble;
  return RP.visit(R);
}

Error BlockPrinter::visit(WallclockRecord &R) {
  CurrentState = State::Preamble;
  return RP.visit(R);
}

Error BlockPrinter::visit(PIDRecord &R) {
  CurrentState = State::Preamble;
  return RP.visit(R);
}

// Metadata printing.
Error BlockPrinter::visit(NewCPUIDRecord &R) {
  if (CurrentState == State::Preamble)
    OS << "\nBody:\n";
  if (CurrentState == State::Function)
    OS << "\nMetadata: ";
  CurrentState = State::Metadata;
  OS << " ";
  auto E = RP.visit(R);
  return E;
}

Error BlockPrinter::visit(TSCWrapRecord &R) {
  if (CurrentState == State::Function)
    OS << "\nMetadata:";
  CurrentState = State::Metadata;
  OS << " ";
  auto E = RP.visit(R);
  return E;
}

// Custom events will be rendered like "function" events.
Error BlockPrinter::visit(CustomEventRecord &R) {
  if (CurrentState == State::Metadata)
    OS << "\n";
  CurrentState = State::CustomEvent;
  OS << "*  ";
  auto E = RP.visit(R);
  return E;
}

Error BlockPrinter::visit(CustomEventRecordV5 &R) {
  if (CurrentState == State::Metadata)
    OS << "\n";
  CurrentState = State::CustomEvent;
  OS << "*  ";
  auto E = RP.visit(R);
  return E;
}

Error BlockPrinter::visit(TypedEventRecord &R) {
  if (CurrentState == State::Metadata)
    OS << "\n";
  CurrentState = State::CustomEvent;
  OS << "*  ";
  auto E = RP.visit(R);
  return E;
}

// Function call printing.
Error BlockPrinter::visit(FunctionRecord &R) {
  if (CurrentState == State::Metadata)
    OS << "\n";
  CurrentState = State::Function;
  OS << "-  ";
  auto E = RP.visit(R);
  return E;
}

Error BlockPrinter::visit(CallArgRecord &R) {
  CurrentState = State::Arg;
  OS << " : ";
  auto E = RP.visit(R);
  return E;
}

Error BlockPrinter::visit(EndBufferRecord &R) {
    CurrentState = State::End;
    OS << " *** ";
    auto E = RP.visit(R);
    return E;
}
