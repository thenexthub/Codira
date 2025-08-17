//===----- ABI.h - ABI related declarations ---------------------*- C++ -*-===//
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
/// Enums/classes describing ABI related information about constructors,
/// destructors and thunks.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_ABI_H
#define LANGUAGE_CORE_BASIC_ABI_H

#include "toolchain/Support/DataTypes.h"
#include <cstring>

namespace language::Core {

/// C++ constructor types.
enum CXXCtorType {
  Ctor_Complete,       ///< Complete object ctor
  Ctor_Base,           ///< Base object ctor
  Ctor_Comdat,         ///< The COMDAT used for ctors
  Ctor_CopyingClosure, ///< Copying closure variant of a ctor
  Ctor_DefaultClosure, ///< Default closure variant of a ctor
};

/// C++ destructor types.
enum CXXDtorType {
    Dtor_Deleting, ///< Deleting dtor
    Dtor_Complete, ///< Complete object dtor
    Dtor_Base,     ///< Base object dtor
    Dtor_Comdat    ///< The COMDAT used for dtors
};

} // end namespace language::Core

#endif
