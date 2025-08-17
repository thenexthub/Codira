//===-- RecordOps.h ---------------------------------------------*- C++ -*-===//
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
//  Operations on records (structs, classes, and unions).
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_RECORDOPS_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_RECORDOPS_H

#include "language/Core/Analysis/FlowSensitive/DataflowEnvironment.h"
#include "language/Core/Analysis/FlowSensitive/StorageLocation.h"

namespace language::Core {
namespace dataflow {

/// Copies a record (struct, class, or union) from `Src` to `Dst`.
///
/// This performs a deep copy, i.e. it copies every field (including synthetic
/// fields) and recurses on fields of record type.
///
/// If there is a `RecordValue` associated with `Dst` in the environment, this
/// function creates a new `RecordValue` and associates it with `Dst`; clients
/// need to be aware of this and must not assume that the `RecordValue`
/// associated with `Dst` remains the same after the call.
///
/// Requirements:
///
///  Either:
///    - `Src` and `Dest` must have the same canonical unqualified type, or
///    - The type of `Src` must be derived from `Dest`, or
///    - The type of `Dest` must be derived from `Src` (in this case, any fields
///      that are only present in `Dest` are not overwritten).
void copyRecord(RecordStorageLocation &Src, RecordStorageLocation &Dst,
                Environment &Env);

/// Returns whether the records `Loc1` and `Loc2` are equal.
///
/// Values for `Loc1` are retrieved from `Env1`, and values for `Loc2` are
/// retrieved from `Env2`. A convenience overload retrieves values for `Loc1`
/// and `Loc2` from the same environment.
///
/// This performs a deep comparison, i.e. it compares every field (including
/// synthetic fields) and recurses on fields of record type. Fields of reference
/// type compare equal if they refer to the same storage location.
///
/// Note on how to interpret the result:
/// - If this returns true, the records are guaranteed to be equal at runtime.
/// - If this returns false, the records may still be equal at runtime; our
///   analysis merely cannot guarantee that they will be equal.
///
/// Requirements:
///
///  `Src` and `Dst` must have the same canonical unqualified type.
bool recordsEqual(const RecordStorageLocation &Loc1, const Environment &Env1,
                  const RecordStorageLocation &Loc2, const Environment &Env2);

inline bool recordsEqual(const RecordStorageLocation &Loc1,
                         const RecordStorageLocation &Loc2,
                         const Environment &Env) {
  return recordsEqual(Loc1, Env, Loc2, Env);
}

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_RECORDOPS_H
