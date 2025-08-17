//===- ConstantInitFuture.h - "Future" constant initializers ----*- C++ -*-===//
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
//
// This class defines the ConstantInitFuture class.  This is split out
// from ConstantInitBuilder.h in order to allow APIs to work with it
// without having to include that entire header.  This is particularly
// important because it is often useful to be able to default-construct
// a future in, say, a default argument.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_CODEGEN_CONSTANTINITFUTURE_H
#define LANGUAGE_CORE_CODEGEN_CONSTANTINITFUTURE_H

#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/IR/Constant.h"

// Forward-declare ConstantInitBuilderBase and give it a
// PointerLikeTypeTraits specialization so that we can safely use it
// in a PointerUnion below.
namespace language::Core {
namespace CodeGen {
class ConstantInitBuilderBase;
}
}
namespace toolchain {
template <>
struct PointerLikeTypeTraits< ::language::Core::CodeGen::ConstantInitBuilderBase*> {
  using T = ::language::Core::CodeGen::ConstantInitBuilderBase*;

  static inline void *getAsVoidPointer(T p) { return p; }
  static inline T getFromVoidPointer(void *p) {return static_cast<T>(p);}
  static constexpr int NumLowBitsAvailable = 2;
};
}

namespace language::Core {
namespace CodeGen {

/// A "future" for a completed constant initializer, which can be passed
/// around independently of any sub-builders (but not the original parent).
class ConstantInitFuture {
  using PairTy = toolchain::PointerUnion<ConstantInitBuilderBase*, toolchain::Constant*>;

  PairTy Data;

  friend class ConstantInitBuilderBase;
  explicit ConstantInitFuture(ConstantInitBuilderBase *builder);

public:
  ConstantInitFuture() {}

  /// A future can be explicitly created from a fixed initializer.
  explicit ConstantInitFuture(toolchain::Constant *initializer) : Data(initializer) {
    assert(initializer && "creating null future");
  }

  /// Is this future non-null?
  explicit operator bool() const { return bool(Data); }

  /// Return the type of the initializer.
  toolchain::Type *getType() const;

  /// Abandon this initializer.
  void abandon();

  /// Install the initializer into a global variable.  This cannot
  /// be called multiple times.
  void installInGlobal(toolchain::GlobalVariable *global);

  void *getOpaqueValue() const { return Data.getOpaqueValue(); }
  static ConstantInitFuture getFromOpaqueValue(void *value) {
    ConstantInitFuture result;
    result.Data = PairTy::getFromOpaqueValue(value);
    return result;
  }
  static constexpr int NumLowBitsAvailable =
      toolchain::PointerLikeTypeTraits<PairTy>::NumLowBitsAvailable;
};

}  // end namespace CodeGen
}  // end namespace language::Core

namespace toolchain {

template <>
struct PointerLikeTypeTraits< ::language::Core::CodeGen::ConstantInitFuture> {
  using T = ::language::Core::CodeGen::ConstantInitFuture;

  static inline void *getAsVoidPointer(T future) {
    return future.getOpaqueValue();
  }
  static inline T getFromVoidPointer(void *p) {
    return T::getFromOpaqueValue(p);
  }
  static constexpr int NumLowBitsAvailable = T::NumLowBitsAvailable;
};

} // end namespace toolchain

#endif
