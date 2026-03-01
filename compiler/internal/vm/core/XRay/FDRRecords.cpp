//===- FDRRecords.cpp -  XRay Flight Data Recorder Mode Records -----------===//
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
// Define types and operations on these types that represent the different kinds
// of records we encounter in XRay flight data recorder mode traces.
//
//===----------------------------------------------------------------------===//
#include "vm/core/XRay/FDRRecords.h"

using namespace vm::core;
using namespace vm::core::xray;

Error BufferExtents::apply(RecordVisitor &V) { return V.visit(*this); }
Error WallclockRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error NewCPUIDRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error TSCWrapRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error CustomEventRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error CallArgRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error PIDRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error NewBufferRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error EndBufferRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error FunctionRecord::apply(RecordVisitor &V) { return V.visit(*this); }
Error CustomEventRecordV5::apply(RecordVisitor &V) { return V.visit(*this); }
Error TypedEventRecord::apply(RecordVisitor &V) { return V.visit(*this); }

StringRef Record::kindToString(RecordKind K) {
  switch (K) {
  case RecordKind::RK_Metadata:
    return "Metadata";
  case RecordKind::RK_Metadata_BufferExtents:
    return "Metadata:BufferExtents";
  case RecordKind::RK_Metadata_WallClockTime:
    return "Metadata:WallClockTime";
  case RecordKind::RK_Metadata_NewCPUId:
    return "Metadata:NewCPUId";
  case RecordKind::RK_Metadata_TSCWrap:
    return "Metadata:TSCWrap";
  case RecordKind::RK_Metadata_CustomEvent:
    return "Metadata:CustomEvent";
  case RecordKind::RK_Metadata_CustomEventV5:
    return "Metadata:CustomEventV5";
  case RecordKind::RK_Metadata_CallArg:
    return "Metadata:CallArg";
  case RecordKind::RK_Metadata_PIDEntry:
    return "Metadata:PIDEntry";
  case RecordKind::RK_Metadata_NewBuffer:
    return "Metadata:NewBuffer";
  case RecordKind::RK_Metadata_EndOfBuffer:
    return "Metadata:EndOfBuffer";
  case RecordKind::RK_Metadata_TypedEvent:
    return "Metadata:TypedEvent";
  case RecordKind::RK_Metadata_LastMetadata:
    return "Metadata:LastMetadata";
  case RecordKind::RK_Function:
    return "Function";
  }
  return "Unknown";
}
