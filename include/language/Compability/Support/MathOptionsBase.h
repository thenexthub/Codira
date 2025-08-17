//===-- language/Compability/Support/MathOptionsBase.h -----------------*- C++ -*-===//
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
/// Options controlling mathematical computations generated in FIR.
/// This is intended to be header-only implementation without extra
/// dependencies so that multiple components can use it to exchange
/// math configuration.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_SUPPORT_MATHOPTIONSBASE_H_
#define LANGUAGE_COMPABILITY_SUPPORT_MATHOPTIONSBASE_H_

namespace language::Compability::common {

class MathOptionsBase {
public:
#define ENUM_MATHOPT(Name, Type, Bits, Default) \
  Type get##Name() const { return static_cast<Type>(Name); } \
  MathOptionsBase &set##Name(Type Value) { \
    Name = static_cast<unsigned>(Value); \
    return *this; \
  }
#include "MathOptionsBase.def"

  MathOptionsBase() {
#define ENUM_MATHOPT(Name, Type, Bits, Default) set##Name(Default);
#include "MathOptionsBase.def"
  }

private:
#define ENUM_MATHOPT(Name, Type, Bits, Default) unsigned Name : Bits;
#include "MathOptionsBase.def"
};

} // namespace language::Compability::common

#endif // FORTRAN_SUPPORT_MATHOPTIONSBASE_H_
