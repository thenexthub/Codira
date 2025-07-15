//===--- TypeWalker.h - Class for walking a Type ----------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_TYPEWALKER_H
#define LANGUAGE_AST_TYPEWALKER_H

#include "language/AST/Type.h"

namespace language {

/// An abstract class used to traverse a Type.
class TypeWalker {
public:
  enum class Action {
    Continue,
    SkipNode,
    Stop
  };

  /// This method is called when first visiting a type before walking into its
  /// children.
  virtual Action walkToTypePre(Type ty) { return Action::Continue; }

  /// This method is called after visiting a type's children.
  virtual Action walkToTypePost(Type ty) { return Action::Continue; }

protected:
  TypeWalker() = default;
  TypeWalker(const TypeWalker &) = default;
  virtual ~TypeWalker() = default;
  
  virtual void anchor();
};

} // end namespace language

#endif
