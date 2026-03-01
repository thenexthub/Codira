//===---------- ObjectTransformLayer.cpp - Object Transform Layer ---------===//
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

#include "vm/core/ExecutionEngine/Orc/ObjectTransformLayer.h"
#include "vm/core/Support/MemoryBuffer.h"

namespace vm::core {
namespace orc {

char ObjectTransformLayer::ID;

using BaseT = RTTIExtends<ObjectTransformLayer, ObjectLayer>;

ObjectTransformLayer::ObjectTransformLayer(ExecutionSession &ES,
                                           ObjectLayer &BaseLayer,
                                           TransformFunction Transform)
    : BaseT(ES), BaseLayer(BaseLayer), Transform(std::move(Transform)) {}

void ObjectTransformLayer::emit(
    std::unique_ptr<MaterializationResponsibility> R,
    std::unique_ptr<MemoryBuffer> O) {
  assert(O && "Module must not be null");

  // If there is a transform set then apply it.
  if (Transform) {
    if (auto TransformedObj = Transform(std::move(O)))
      O = std::move(*TransformedObj);
    else {
      R->failMaterialization();
      getExecutionSession().reportError(TransformedObj.takeError());
      return;
    }
  }

  BaseLayer.emit(std::move(R), std::move(O));
}

} // End namespace orc.
} // End namespace vm::core.
