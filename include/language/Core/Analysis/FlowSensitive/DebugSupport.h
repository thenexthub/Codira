//===-- DebugSupport.h ------------------------------------------*- C++ -*-===//
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
//  This file defines functions which generate more readable forms of data
//  structures used in the dataflow analyses, for debugging purposes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_DEBUGSUPPORT_H_
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_DEBUGSUPPORT_H_

#include <string>
#include <vector>

#include "language/Core/Analysis/FlowSensitive/Solver.h"
#include "language/Core/Analysis/FlowSensitive/Value.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
namespace dataflow {

/// Returns a string representation of a value kind.
toolchain::StringRef debugString(Value::Kind Kind);

/// Returns a string representation of the result status of a SAT check.
toolchain::StringRef debugString(Solver::Result::Status Status);

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_DEBUGSUPPORT_H_
