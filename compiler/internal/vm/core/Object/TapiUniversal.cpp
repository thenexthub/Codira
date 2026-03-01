//===- TapiUniversal.cpp --------------------------------------------------===//
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
// This file defines the Text-based Dynamic Library Stub format.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Object/TapiUniversal.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Object/TapiFile.h"
#include "vm/core/TextAPI/TextAPIReader.h"

using namespace vm::core;
using namespace MachO;
using namespace object;

TapiUniversal::TapiUniversal(MemoryBufferRef Source, Error &Err)
    : Binary(ID_TapiUniversal, Source) {
  Expected<std::unique_ptr<InterfaceFile>> Result = TextAPIReader::get(Source);
  ErrorAsOutParameter ErrAsOuParam(Err);
  if (!Result) {
    Err = Result.takeError();
    return;
  }
  ParsedFile = std::move(Result.get());

  auto FlattenObjectInfo = [this](const auto &File,
                                  std::optional<size_t> DocIdx = std::nullopt) {
    StringRef Name = File->getInstallName();
    for (const Architecture Arch : File->getArchitectures())
      Libraries.emplace_back(Library({Name, Arch, DocIdx}));
  };
  FlattenObjectInfo(ParsedFile);
  // Get inlined documents from tapi file.
  size_t DocIdx = 0;
  for (const std::shared_ptr<InterfaceFile> &File : ParsedFile->documents())
    FlattenObjectInfo(File, DocIdx++);
}

TapiUniversal::~TapiUniversal() = default;

Expected<std::unique_ptr<TapiFile>>
TapiUniversal::ObjectForArch::getAsObjectFile() const {
  const auto &InlinedDocuments = Parent->ParsedFile->documents();
  const Library &CurrLib = Parent->Libraries[Index];
  assert(
      (isTopLevelLib() || (CurrLib.DocumentIdx.has_value() &&
                           (InlinedDocuments.size() > *CurrLib.DocumentIdx))) &&
      "Index into documents exceeds the container for them");
  InterfaceFile *IF = isTopLevelLib()
                          ? Parent->ParsedFile.get()
                          : InlinedDocuments[*CurrLib.DocumentIdx].get();
  return std::make_unique<TapiFile>(Parent->getMemoryBufferRef(), *IF,
                                    CurrLib.Arch);
}

Expected<std::unique_ptr<TapiUniversal>>
TapiUniversal::create(MemoryBufferRef Source) {
  Error Err = Error::success();
  std::unique_ptr<TapiUniversal> Ret(new TapiUniversal(Source, Err));
  if (Err)
    return std::move(Err);
  return std::move(Ret);
}
