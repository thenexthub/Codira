//===----------------------- OrcRTBootstrap.h -------------------*- C++ -*-===//
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
//
// OrcRTPrelinkImpl provides functions that should be linked into the executor
// to bootstrap common JIT functionality (e.g. memory allocation and memory
// access).
//
// Call rt_impl::addTo to add these functions to a bootstrap symbols map.
//
// FIXME: The functionality in this file should probably be moved to an ORC
// runtime bootstrap library in compiler-rt.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_EXECUTIONENGINE_ORC_TARGETPROCESS_ORCRTBOOTSTRAP_H
#define LIB_EXECUTIONENGINE_ORC_TARGETPROCESS_ORCRTBOOTSTRAP_H

#include "vm/core/ADT/StringMap.h"
#include "vm/core/ExecutionEngine/Orc/Shared/ExecutorAddress.h"

namespace vm::core {
namespace orc {
namespace rt_bootstrap {

void addTo(StringMap<ExecutorAddr> &M);

} // namespace rt_bootstrap
} // end namespace orc
} // end namespace vm::core

#endif // LIB_EXECUTIONENGINE_ORC_TARGETPROCESS_ORCRTBOOTSTRAP_H
