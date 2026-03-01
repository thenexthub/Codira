//===- RecordPrinter.cpp - FDR Record Printer -----------------------------===//
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
#include "vm/core/XRay/RecordPrinter.h"

#include "vm/core/Support/FormatVariadic.h"

using namespace vm::core;
using namespace vm::core::xray;

Error RecordPrinter::visit(BufferExtents &R) {
  OS << formatv("<Buffer: size = {0} bytes>", R.size()) << Delim;
  return Error::success();
}

Error RecordPrinter::visit(WallclockRecord &R) {
  OS << formatv("<Wall Time: seconds = {0}.{1,0+6}>", R.seconds(), R.nanos())
     << Delim;
  return Error::success();
}

Error RecordPrinter::visit(NewCPUIDRecord &R) {
  OS << formatv("<CPU: id = {0}, tsc = {1}>", R.cpuid(), R.tsc()) << Delim;
  return Error::success();
}

Error RecordPrinter::visit(TSCWrapRecord &R) {
  OS << formatv("<TSC Wrap: base = {0}>", R.tsc()) << Delim;
  return Error::success();
}

Error RecordPrinter::visit(CustomEventRecord &R) {
  OS << formatv(
            "<Custom Event: tsc = {0}, cpu = {1}, size = {2}, data = '{3}'>",
            R.tsc(), R.cpu(), R.size(), R.data())
     << Delim;
  return Error::success();
}

Error RecordPrinter::visit(CustomEventRecordV5 &R) {
  OS << formatv("<Custom Event: delta = +{0}, size = {1}, data = '{2}'>",
                R.delta(), R.size(), R.data())
     << Delim;
  return Error::success();
}

Error RecordPrinter::visit(TypedEventRecord &R) {
  OS << formatv(
            "<Typed Event: delta = +{0}, type = {1}, size = {2}, data = '{3}'",
            R.delta(), R.eventType(), R.size(), R.data())
     << Delim;
  return Error::success();
}

Error RecordPrinter::visit(CallArgRecord &R) {
  OS << formatv("<Call Argument: data = {0} (hex = {0:x})>", R.arg()) << Delim;
  return Error::success();
}

Error RecordPrinter::visit(PIDRecord &R) {
  OS << formatv("<PID: {0}>", R.pid()) << Delim;
  return Error::success();
}

Error RecordPrinter::visit(NewBufferRecord &R) {
  OS << formatv("<Thread ID: {0}>", R.tid()) << Delim;
  return Error::success();
}

Error RecordPrinter::visit(EndBufferRecord &R) {
  OS << "<End of Buffer>" << Delim;
  return Error::success();
}

Error RecordPrinter::visit(FunctionRecord &R) {
  // FIXME: Support symbolization here?
  switch (R.recordType()) {
  case RecordTypes::ENTER:
    OS << formatv("<Function Enter: #{0} delta = +{1}>", R.functionId(),
                  R.delta());
    break;
  case RecordTypes::ENTER_ARG:
    OS << formatv("<Function Enter With Arg: #{0} delta = +{1}>",
                  R.functionId(), R.delta());
    break;
  case RecordTypes::EXIT:
    OS << formatv("<Function Exit: #{0} delta = +{1}>", R.functionId(),
                  R.delta());
    break;
  case RecordTypes::TAIL_EXIT:
    OS << formatv("<Function Tail Exit: #{0} delta = +{1}>", R.functionId(),
                  R.delta());
    break;
  case RecordTypes::CUSTOM_EVENT:
  case RecordTypes::TYPED_EVENT:
    // TODO: Flag as a bug?
    break;
  }
  OS << Delim;
  return Error::success();
}
