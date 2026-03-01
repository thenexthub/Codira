//===----------------------------------------------------------------------===//
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
///
/// \file
/// This file provides the basic framework for Telemetry.
/// Refer to its documentation at toolchain/docs/Telemetry.rst for more details.
//===---------------------------------------------------------------------===//

#include "vm/core/Telemetry/Telemetry.h"

namespace vm::core {
namespace telemetry {

void TelemetryInfo::serialize(Serializer &serializer) const {
  serializer.write("SessionId", SessionId);
}

Error Manager::dispatch(TelemetryInfo *Entry) {
  assert(Config::BuildTimeEnableTelemetry &&
         "Telemetry should have been enabled");
  if (Error Err = preDispatch(Entry))
    return Err;

  Error AllErrs = Error::success();
  for (auto &Dest : Destinations) {
    AllErrs = joinErrors(std::move(AllErrs), Dest->receiveEntry(Entry));
  }
  return AllErrs;
}

void Manager::addDestination(std::unique_ptr<Destination> Dest) {
  Destinations.push_back(std::move(Dest));
}

Error Manager::preDispatch(TelemetryInfo *Entry) { return Error::success(); }

} // namespace telemetry
} // namespace vm::core
