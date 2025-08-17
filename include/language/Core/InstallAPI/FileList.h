//===- InstallAPI/FileList.h ------------------------------------*- C++ -*-===//
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
/// The JSON file list parser is used to communicate input to InstallAPI.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INSTALLAPI_FILELIST_H
#define LANGUAGE_CORE_INSTALLAPI_FILELIST_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/InstallAPI/HeaderFile.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/MemoryBuffer.h"

namespace language::Core {
namespace installapi {

class FileListReader {
public:
  /// Decode JSON input and append header input into destination container.
  /// Headers are loaded in the order they appear in the JSON input.
  ///
  /// \param InputBuffer JSON input data.
  /// \param Destination Container to load headers into.
  /// \param FM Optional File Manager to validate input files exist.
  static toolchain::Error
  loadHeaders(std::unique_ptr<toolchain::MemoryBuffer> InputBuffer,
              HeaderSeq &Destination, language::Core::FileManager *FM = nullptr);

  FileListReader() = delete;
};

} // namespace installapi
} // namespace language::Core

#endif // LANGUAGE_CORE_INSTALLAPI_FILELIST_H
