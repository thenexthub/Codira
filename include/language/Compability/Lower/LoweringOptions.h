//===- LoweringOptions.h ----------------------------------------*- C++ -*-===//
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
///
/// \file
/// Options controlling lowering of front-end fragments to the FIR dialect
/// of MLIR
///
//===----------------------------------------------------------------------===//

#ifndef FLANG_LOWER_LOWERINGOPTIONS_H
#define FLANG_LOWER_LOWERINGOPTIONS_H

#include "language/Compability/Support/MathOptionsBase.h"

namespace language::Compability::lower {

class LoweringOptionsBase {
public:
#define LOWERINGOPT(Name, Bits, Default) unsigned Name : Bits;
#define ENUM_LOWERINGOPT(Name, Type, Bits, Default)
#include "language/Compability/Lower/LoweringOptions.def"

protected:
#define LOWERINGOPT(Name, Bits, Default)
#define ENUM_LOWERINGOPT(Name, Type, Bits, Default) unsigned Name : Bits;
#include "language/Compability/Lower/LoweringOptions.def"
};

class LoweringOptions : public LoweringOptionsBase {

public:
#define LOWERINGOPT(Name, Bits, Default)
#define ENUM_LOWERINGOPT(Name, Type, Bits, Default)                            \
  Type get##Name() const { return static_cast<Type>(Name); }                   \
  LoweringOptions &set##Name(Type Value) {                                     \
    Name = static_cast<unsigned>(Value);                                       \
    return *this;                                                              \
  }
#include "language/Compability/Lower/LoweringOptions.def"

  LoweringOptions();

  const language::Compability::common::MathOptionsBase &getMathOptions() const {
    return MathOptions;
  }

  language::Compability::common::MathOptionsBase &getMathOptions() { return MathOptions; }

private:
  /// Options for handling/optimizing mathematical computations.
  language::Compability::common::MathOptionsBase MathOptions;
};

} // namespace language::Compability::lower

#endif // FLANG_LOWER_LOWERINGOPTIONS_H
