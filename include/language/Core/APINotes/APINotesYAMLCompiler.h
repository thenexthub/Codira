//===-- APINotesYAMLCompiler.h - API Notes YAML Format Reader ---*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_APINOTES_APINOTESYAMLCOMPILER_H
#define LANGUAGE_CORE_APINOTES_APINOTESYAMLCOMPILER_H

#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/SourceMgr.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {
class FileEntry;
} // namespace language::Core

namespace language::Core {
namespace api_notes {
/// Parses the APINotes YAML content and writes the representation back to the
/// specified stream.  This provides a means of testing the YAML processing of
/// the APINotes format.
bool parseAndDumpAPINotes(toolchain::StringRef YI, toolchain::raw_ostream &OS);

/// Converts API notes from YAML format to binary format.
bool compileAPINotes(toolchain::StringRef YAMLInput, const FileEntry *SourceFile,
                     toolchain::raw_ostream &OS,
                     toolchain::SourceMgr::DiagHandlerTy DiagHandler = nullptr,
                     void *DiagHandlerCtxt = nullptr);
} // namespace api_notes
} // namespace language::Core

#endif
