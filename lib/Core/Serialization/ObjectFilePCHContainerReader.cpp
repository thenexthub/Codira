//===--- ObjectFilePCHContainerReader.cpp ---------------------------------===//
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

#include "language/Core/Serialization/ObjectFilePCHContainerReader.h"
#include "toolchain/Object/COFF.h"
#include "toolchain/Object/ObjectFile.h"

using namespace language::Core;

ArrayRef<StringRef> ObjectFilePCHContainerReader::getFormats() const {
  static StringRef Formats[] = {"obj", "raw"};
  return Formats;
}

StringRef
ObjectFilePCHContainerReader::ExtractPCH(toolchain::MemoryBufferRef Buffer) const {
  StringRef PCH;
  auto OFOrErr = toolchain::object::ObjectFile::createObjectFile(Buffer);
  if (OFOrErr) {
    auto &OF = OFOrErr.get();
    bool IsCOFF = isa<toolchain::object::COFFObjectFile>(*OF);
    // Find the clang AST section in the container.
    for (auto &Section : OF->sections()) {
      StringRef Name;
      if (Expected<StringRef> NameOrErr = Section.getName())
        Name = *NameOrErr;
      else
        consumeError(NameOrErr.takeError());

      if ((!IsCOFF && Name == "__clangast") || (IsCOFF && Name == "clangast")) {
        if (Expected<StringRef> E = Section.getContents())
          return *E;
        else {
          handleAllErrors(E.takeError(), [&](const toolchain::ErrorInfoBase &EIB) {
            EIB.log(toolchain::errs());
          });
          return "";
        }
      }
    }
  }
  handleAllErrors(OFOrErr.takeError(), [&](const toolchain::ErrorInfoBase &EIB) {
    if (EIB.convertToErrorCode() ==
        toolchain::object::object_error::invalid_file_type)
      // As a fallback, treat the buffer as a raw AST.
      PCH = Buffer.getBuffer();
    else
      EIB.log(toolchain::errs());
  });
  return PCH;
}
