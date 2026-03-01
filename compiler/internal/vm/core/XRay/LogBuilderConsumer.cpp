//===- FDRRecordConsumer.h - XRay Flight Data Recorder Mode Records -------===//
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
#include "vm/core/XRay/FDRRecordConsumer.h"

using namespace vm::core;
using namespace vm::core::xray;

Error LogBuilderConsumer::consume(std::unique_ptr<Record> R) {
  if (!R)
    return createStringError(
        std::make_error_code(std::errc::invalid_argument),
        "Must not call RecordConsumer::consume() with a null pointer.");
  Records.push_back(std::move(R));
  return Error::success();
}

Error PipelineConsumer::consume(std::unique_ptr<Record> R) {
  if (!R)
    return createStringError(
        std::make_error_code(std::errc::invalid_argument),
        "Must not call RecordConsumer::consume() with a null pointer.");

  // We apply all of the visitors in order, and concatenate errors
  // appropriately.
  Error Result = Error::success();
  for (auto *V : Visitors)
    Result = joinErrors(std::move(Result), R->apply(*V));
  return Result;
}
