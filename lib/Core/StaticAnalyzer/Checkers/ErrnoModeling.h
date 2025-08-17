//=== ErrnoModeling.h - Tracking value of 'errno'. -----------------*- C++ -*-//
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
// Defines inter-checker API for using the system value 'errno'.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_ERRNOMODELING_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_ERRNOMODELING_H

#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include <optional>

namespace language::Core {
namespace ento {
namespace errno_modeling {

/// Describe how reads and writes of \c errno are handled by the checker.
enum ErrnoCheckState : unsigned {
  /// We do not know anything about 'errno'.
  /// Read and write is always allowed.
  Irrelevant = 0,

  /// Value of 'errno' should be checked to find out if a previous function call
  /// has failed.
  /// When this state is set \c errno must be read by the program before a next
  /// standard function call or other overwrite of \c errno follows, otherwise
  /// a bug report is emitted.
  MustBeChecked = 1,

  /// Value of 'errno' is not allowed to be read, it can contain an unspecified
  /// value.
  /// When this state is set \c errno is not allowed to be read by the program
  /// until it is overwritten or invalidated.
  MustNotBeChecked = 2
};

/// Returns the value of 'errno', if 'errno' was found in the AST.
std::optional<SVal> getErrnoValue(ProgramStateRef State);

/// Returns the errno check state, \c Errno_Irrelevant if 'errno' was not found
/// (this is not the only case for that value).
ErrnoCheckState getErrnoState(ProgramStateRef State);

/// Returns the location that points to the \c MemoryRegion where the 'errno'
/// value is stored. Returns \c std::nullopt if 'errno' was not found. Otherwise
/// it always returns a valid memory region in the system global memory space.
std::optional<Loc> getErrnoLoc(ProgramStateRef State);

/// Set value of 'errno' to any SVal, if possible.
/// The errno check state is set always when the 'errno' value is set.
ProgramStateRef setErrnoValue(ProgramStateRef State,
                              const LocationContext *LCtx, SVal Value,
                              ErrnoCheckState EState);

/// Set value of 'errno' to a concrete (signed) integer, if possible.
/// The errno check state is set always when the 'errno' value is set.
ProgramStateRef setErrnoValue(ProgramStateRef State, CheckerContext &C,
                              uint64_t Value, ErrnoCheckState EState);

/// Set the errno check state, do not modify the errno value.
ProgramStateRef setErrnoState(ProgramStateRef State, ErrnoCheckState EState);

/// Clear state of errno (make it irrelevant).
ProgramStateRef clearErrnoState(ProgramStateRef State);

/// Determine if `Call` is a call to an internal function that returns the
/// location of `errno` (in environments where errno is accessed this way).
bool isErrnoLocationCall(const CallEvent &Call);

/// Create a NoteTag that displays the message if the 'errno' memory region is
/// marked as interesting, and resets the interestingness.
const NoteTag *getErrnoNoteTag(CheckerContext &C, const std::string &Message);

/// Set errno state for the common case when a standard function is successful.
/// Set \c ErrnoCheckState to \c MustNotBeChecked (the \c errno value is not
/// affected).
ProgramStateRef setErrnoForStdSuccess(ProgramStateRef State, CheckerContext &C);

/// Set errno state for the common case when a standard function fails.
/// Set \c errno value to be not equal to zero and \c ErrnoCheckState to
/// \c Irrelevant . The irrelevant errno state ensures that no related bug
/// report is emitted later and no note tag is needed.
/// \arg \c ErrnoSym Value to be used for \c errno and constrained to be
/// non-zero.
ProgramStateRef setErrnoForStdFailure(ProgramStateRef State, CheckerContext &C,
                                      NonLoc ErrnoSym);

/// Set errno state for the common case when a standard function indicates
/// failure only by \c errno. Sets \c ErrnoCheckState to \c MustBeChecked, and
/// invalidates the errno region (clear of previous value).
/// \arg \c Elem CFG Element that causes invalidation of \c errno.
ProgramStateRef setErrnoStdMustBeChecked(ProgramStateRef State,
                                         CheckerContext &C,
                                         ConstCFGElementRef Elem);

} // namespace errno_modeling
} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_ERRNOMODELING_H
