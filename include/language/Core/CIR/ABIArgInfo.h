//==-- ABIArgInfo.h - Abstract info regarding ABI-specific arguments -------==//
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
// Defines ABIArgInfo and associated types used by CIR to track information
// regarding ABI-coerced types for function arguments and return values. This
// was moved to the common library as it might be used by both CIRGen and
// passes.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CIR_ABIARGINFO_H
#define CLANG_CIR_ABIARGINFO_H

#include "mlir/IR/Types.h"
#include "language/Core/CIR/MissingFeatures.h"

namespace cir {

class ABIArgInfo {
public:
  enum Kind : uint8_t {
    /// Pass the argument directly using the normal converted CIR type,
    /// or by coercing to another specified type stored in 'CoerceToType'). If
    /// an offset is specified (in UIntData), then the argument passed is offset
    /// by some number of bytes in the memory representation. A dummy argument
    /// is emitted before the real argument if the specified type stored in
    /// "PaddingType" is not zero.
    Direct,

    /// Ignore the argument (treat as void). Useful for void and empty
    /// structs.
    Ignore,

    // TODO: more argument kinds will be added as the upstreaming proceeds.
  };

private:
  mlir::Type typeData;
  struct DirectAttrInfo {
    unsigned offset;
    unsigned align;
  };
  union {
    DirectAttrInfo directAttr;
  };
  Kind theKind;

public:
  ABIArgInfo(Kind k = Direct) : directAttr{0, 0}, theKind(k) {}

  static ABIArgInfo getDirect(mlir::Type ty = nullptr) {
    ABIArgInfo info(Direct);
    info.setCoerceToType(ty);
    assert(!cir::MissingFeatures::abiArgInfo());
    return info;
  }

  static ABIArgInfo getIgnore() { return ABIArgInfo(Ignore); }

  Kind getKind() const { return theKind; }
  bool isDirect() const { return theKind == Direct; }
  bool isIgnore() const { return theKind == Ignore; }

  bool canHaveCoerceToType() const {
    assert(!cir::MissingFeatures::abiArgInfo());
    return isDirect();
  }

  unsigned getDirectOffset() const {
    assert(!cir::MissingFeatures::abiArgInfo());
    return directAttr.offset;
  }

  mlir::Type getCoerceToType() const {
    assert(canHaveCoerceToType() && "invalid kind!");
    return typeData;
  }

  void setCoerceToType(mlir::Type ty) {
    assert(canHaveCoerceToType() && "invalid kind!");
    typeData = ty;
  }
};

} // namespace cir

#endif // CLANG_CIR_ABIARGINFO_H
