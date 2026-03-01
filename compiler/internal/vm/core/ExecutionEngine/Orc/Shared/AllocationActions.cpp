//===----- AllocationActions.gpp -- JITLink allocation support calls  -----===//
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

#include "vm/core/ExecutionEngine/Orc/Shared/AllocationActions.h"

namespace vm::core {
namespace orc {
namespace shared {

Expected<std::vector<WrapperFunctionCall>>
runFinalizeActions(AllocActions &AAs) {
  std::vector<WrapperFunctionCall> DeallocActions;
  DeallocActions.reserve(numDeallocActions(AAs));

  for (auto &AA : AAs) {
    if (AA.Finalize)
      if (auto Err = AA.Finalize.runWithSPSRetErrorMerged())
        return joinErrors(std::move(Err), runDeallocActions(DeallocActions));

    if (AA.Dealloc)
      DeallocActions.push_back(std::move(AA.Dealloc));
  }

  AAs.clear();
  return DeallocActions;
}

Error runDeallocActions(ArrayRef<WrapperFunctionCall> DAs) {
  Error Err = Error::success();
  while (!DAs.empty()) {
    Err = joinErrors(std::move(Err), DAs.back().runWithSPSRetErrorMerged());
    DAs = DAs.drop_back();
  }
  return Err;
}

} // namespace shared
} // namespace orc
} // namespace vm::core
