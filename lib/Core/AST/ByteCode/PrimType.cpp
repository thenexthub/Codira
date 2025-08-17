//===--- PrimType.cpp - Types for the constexpr VM --------------*- C++ -*-===//
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

#include "PrimType.h"
#include "Boolean.h"
#include "FixedPoint.h"
#include "Floating.h"
#include "IntegralAP.h"
#include "MemberPointer.h"
#include "Pointer.h"

using namespace language::Core;
using namespace language::Core::interp;

namespace language::Core {
namespace interp {

size_t primSize(PrimType Type) {
  TYPE_SWITCH(Type, return sizeof(T));
  toolchain_unreachable("not a primitive type");
}

} // namespace interp
} // namespace language::Core
