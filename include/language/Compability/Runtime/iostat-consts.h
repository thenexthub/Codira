//===-- language/Compability/Runtime/iostat-consts.h -------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_IOSTAT_CONSTS_H_
#define LANGUAGE_COMPABILITY_RUNTIME_IOSTAT_CONSTS_H_

#include "language/Compability/Common/api-attrs.h"
#include "language/Compability/Runtime/magic-numbers.h"

namespace language::Compability::runtime::io {

// The value of IOSTAT= is zero when no error, end-of-record,
// or end-of-file condition has arisen; errors are positive values.
// (See 12.11.5 in Fortran 2018 for the complete requirements;
// these constants must match the values of their corresponding
// named constants in the predefined module ISO_FORTRAN_ENV, so
// they're actually defined in another magic-numbers.h header file
// so that they can be included both here and there.)
enum Iostat {
  IostatOk = 0, // no error, EOF, or EOR condition

  // These error codes are required by Fortran (see 12.10.2.16-17) to be
  // negative integer values
  IostatEnd = FORTRAN_RUNTIME_IOSTAT_END, // end-of-file on input & no error
  // End-of-record on non-advancing input, no EOF or error
  IostatEor = FORTRAN_RUNTIME_IOSTAT_EOR,

  // This value is also required to be negative (12.11.5 bullet 6).
  // It signifies a FLUSH statement on an unflushable unit.
  IostatUnflushable = FORTRAN_RUNTIME_IOSTAT_FLUSH,

  // Other errors are positive.  We use "errno" values unchanged.
  // This error is exported in ISO_Fortran_env.
  IostatInquireInternalUnit = FORTRAN_RUNTIME_IOSTAT_INQUIRE_INTERNAL_UNIT,

  // The remaining error codes are not exported.
  IostatGenericError = 1001, // see IOMSG= for details
  IostatRecordWriteOverrun,
  IostatRecordReadOverrun,
  IostatInternalWriteOverrun,
  IostatErrorInFormat,
  IostatErrorInKeyword,
  IostatEndfileDirect,
  IostatEndfileUnwritable,
  IostatOpenBadRecl,
  IostatOpenUnknownSize,
  IostatOpenBadAppend,
  IostatWriteToReadOnly,
  IostatReadFromWriteOnly,
  IostatBackspaceNonSequential,
  IostatBackspaceAtFirstRecord,
  IostatRewindNonSequential,
  IostatWriteAfterEndfile,
  IostatFormattedIoOnUnformattedUnit,
  IostatUnformattedIoOnFormattedUnit,
  IostatListIoOnDirectAccessUnit,
  IostatUnformattedChildOnFormattedParent,
  IostatFormattedChildOnUnformattedParent,
  IostatChildInputFromOutputParent,
  IostatChildOutputToInputParent,
  IostatShortRead,
  IostatMissingTerminator,
  IostatBadUnformattedRecord,
  IostatUTF8Decoding,
  IostatUnitOverflow,
  IostatBadRealInput,
  IostatBadScaleFactor,
  IostatBadAsynchronous,
  IostatBadWaitUnit,
  IostatBOZInputOverflow,
  IostatIntegerInputOverflow,
  IostatRealInputOverflow,
  IostatOpenAlreadyConnected,
  IostatCannotReposition,
  IostatBadWaitId,
  IostatTooManyAsyncOps,
  IostatBadBackspaceUnit,
  IostatBadUnitNumber,
  IostatBadFlushUnit,
  IostatBadOpOnChildUnit,
  IostatBadNewUnit,
  IostatBadListDirectedInputSeparator,
  IostatNonExternalDefinedUnformattedIo,
};

} // namespace language::Compability::runtime::io

#endif // FORTRAN_RUNTIME_IOSTAT_CONSTS_H_
