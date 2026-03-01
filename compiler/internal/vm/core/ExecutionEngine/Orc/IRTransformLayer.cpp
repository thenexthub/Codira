//===-------------- IRTransformLayer.cpp - IR Transform Layer -------------===//
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

#include "vm/core/ExecutionEngine/Orc/IRTransformLayer.h"

namespace vm::core {
namespace orc {

IRTransformLayer::IRTransformLayer(ExecutionSession &ES, IRLayer &BaseLayer,
                                   TransformFunction Transform)
    : IRLayer(ES, BaseLayer.getManglingOptions()), BaseLayer(BaseLayer),
      Transform(std::move(Transform)) {}

void IRTransformLayer::emit(std::unique_ptr<MaterializationResponsibility> R,
                            ThreadSafeModule TSM) {
  assert(TSM && "Module must not be null");

  if (auto TransformedTSM = Transform(std::move(TSM), *R))
    BaseLayer.emit(std::move(R), std::move(*TransformedTSM));
  else {
    R->failMaterialization();
    getExecutionSession().reportError(TransformedTSM.takeError());
  }
}

} // End namespace orc.
} // End namespace vm::core.
