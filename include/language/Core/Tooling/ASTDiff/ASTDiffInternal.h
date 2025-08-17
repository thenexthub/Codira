//===- ASTDiffInternal.h --------------------------------------*- C++ -*- -===//
//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LANGUAGE_CORE_TOOLING_ASTDIFF_ASTDIFFINTERNAL_H
#define LANGUAGE_CORE_TOOLING_ASTDIFF_ASTDIFFINTERNAL_H

#include "language/Core/AST/ASTTypeTraits.h"

namespace language::Core {
namespace diff {

using DynTypedNode = DynTypedNode;

/// Within a tree, this identifies a node by its preorder offset.
struct NodeId {
private:
  static constexpr int InvalidNodeId = -1;

public:
  int Id;

  NodeId() : Id(InvalidNodeId) {}
  NodeId(int Id) : Id(Id) {}

  operator int() const { return Id; }
  NodeId &operator++() { return ++Id, *this; }
  NodeId &operator--() { return --Id, *this; }
  // Support defining iterators on NodeId.
  NodeId &operator*() { return *this; }

  bool isValid() const { return Id != InvalidNodeId; }
  bool isInvalid() const { return Id == InvalidNodeId; }
};

} // end namespace diff
} // end namespace language::Core
#endif
