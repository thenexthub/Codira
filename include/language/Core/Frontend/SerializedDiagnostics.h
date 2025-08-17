//===--- SerializedDiagnostics.h - Common data for serialized diagnostics -===//
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

#ifndef LANGUAGE_CORE_FRONTEND_SERIALIZEDDIAGNOSTICS_H
#define LANGUAGE_CORE_FRONTEND_SERIALIZEDDIAGNOSTICS_H

#include "toolchain/Bitstream/BitCodes.h"

namespace language::Core {
namespace serialized_diags {

enum BlockIDs {
  /// A top-level block which represents any meta data associated
  /// with the diagostics, including versioning of the format.
  BLOCK_META = toolchain::bitc::FIRST_APPLICATION_BLOCKID,

  /// The this block acts as a container for all the information
  /// for a specific diagnostic.
  BLOCK_DIAG
};

enum RecordIDs {
  RECORD_VERSION = 1,
  RECORD_DIAG,
  RECORD_SOURCE_RANGE,
  RECORD_DIAG_FLAG,
  RECORD_CATEGORY,
  RECORD_FILENAME,
  RECORD_FIXIT,
  RECORD_FIRST = RECORD_VERSION,
  RECORD_LAST = RECORD_FIXIT
};

/// A stable version of DiagnosticIDs::Level.
///
/// Do not change the order of values in this enum, and please increment the
/// serialized diagnostics version number when you add to it.
enum Level {
  Ignored = 0,
  Note,
  Warning,
  Error,
  Fatal,
  Remark
};

/// The serialized diagnostics version number.
enum { VersionNumber = 2 };

} // end serialized_diags namespace
} // end clang namespace

#endif
