//===- BlockIndexer.cpp - FDR Block Indexing VIsitor ----------------------===//
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
// An implementation of the RecordVisitor which generates a mapping between a
// thread and a range of records representing a block.
//
//===----------------------------------------------------------------------===//
#include "vm/core/XRay/BlockIndexer.h"

using namespace vm::core;
using namespace vm::core::xray;

Error BlockIndexer::visit(BufferExtents &) { return Error::success(); }

Error BlockIndexer::visit(WallclockRecord &R) {
  CurrentBlock.Records.push_back(&R);
  CurrentBlock.WallclockTime = &R;
  return Error::success();
}

Error BlockIndexer::visit(NewCPUIDRecord &R) {
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(TSCWrapRecord &R) {
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(CustomEventRecord &R) {
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(CustomEventRecordV5 &R) {
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(TypedEventRecord &R) {
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(CallArgRecord &R) {
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(PIDRecord &R) {
  CurrentBlock.ProcessID = R.pid();
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(NewBufferRecord &R) {
  if (!CurrentBlock.Records.empty())
    if (auto E = flush())
      return E;

  CurrentBlock.ThreadID = R.tid();
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(EndBufferRecord &R) {
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::visit(FunctionRecord &R) {
  CurrentBlock.Records.push_back(&R);
  return Error::success();
}

Error BlockIndexer::flush() {
  Indices[{CurrentBlock.ProcessID, CurrentBlock.ThreadID}].push_back(
      {CurrentBlock.ProcessID, CurrentBlock.ThreadID,
       CurrentBlock.WallclockTime, std::move(CurrentBlock.Records)});
  CurrentBlock.ProcessID = 0;
  CurrentBlock.ThreadID = 0;
  CurrentBlock.Records = {};
  CurrentBlock.WallclockTime = nullptr;
  return Error::success();
}
