//===- RemarkLinker.cpp ---------------------------------------------------===//
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
// This file provides an implementation of the remark linker.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Remarks/RemarkLinker.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/Object/SymbolicFile.h"
#include "vm/core/Remarks/RemarkParser.h"
#include "vm/core/Remarks/RemarkSerializer.h"
#include "vm/core/Support/Error.h"
#include <optional>

using namespace vm::core;
using namespace vm::core::remarks;

namespace vm::core {
class raw_ostream;
}

static Expected<StringRef>
getRemarksSectionName(const object::ObjectFile &Obj) {
  if (Obj.isMachO())
    return StringRef("__remarks");
  // ELF -> .remarks, but there is no ELF support at this point.
  return createStringError(std::errc::illegal_byte_sequence,
                           "Unsupported file format.");
}

Expected<std::optional<StringRef>>
toolchain::remarks::getRemarksSectionContents(const object::ObjectFile &Obj) {
  Expected<StringRef> SectionName = getRemarksSectionName(Obj);
  if (!SectionName)
    return SectionName.takeError();

  for (const object::SectionRef &Section : Obj.sections()) {
    Expected<StringRef> MaybeName = Section.getName();
    if (!MaybeName)
      return MaybeName.takeError();
    if (*MaybeName != *SectionName)
      continue;

    if (Expected<StringRef> Contents = Section.getContents())
      return *Contents;
    else
      return Contents.takeError();
  }
  return std::optional<StringRef>{};
}

Remark &RemarkLinker::keep(std::unique_ptr<Remark> Remark) {
  StrTab.internalize(*Remark);
  auto Inserted = Remarks.insert(std::move(Remark));
  return **Inserted.first;
}

void RemarkLinker::setExternalFilePrependPath(StringRef PrependPathIn) {
  PrependPath = std::string(PrependPathIn);
}

Error RemarkLinker::link(StringRef Buffer, Format RemarkFormat) {
  Expected<std::unique_ptr<RemarkParser>> MaybeParser =
      createRemarkParserFromMeta(
          RemarkFormat, Buffer,
          PrependPath ? std::make_optional<StringRef>(*PrependPath)
                      : std::nullopt);
  if (!MaybeParser)
    return MaybeParser.takeError();

  RemarkParser &Parser = **MaybeParser;

  while (true) {
    Expected<std::unique_ptr<Remark>> Next = Parser.next();
    if (Error E = Next.takeError()) {
      if (E.isA<EndOfFileError>()) {
        consumeError(std::move(E));
        break;
      }
      return E;
    }

    assert(*Next != nullptr);

    if (shouldKeepRemark(**Next))
      keep(std::move(*Next));
  }
  return Error::success();
}

Error RemarkLinker::link(const object::ObjectFile &Obj, Format RemarkFormat) {
  Expected<std::optional<StringRef>> SectionOrErr =
      getRemarksSectionContents(Obj);
  if (!SectionOrErr)
    return SectionOrErr.takeError();

  if (std::optional<StringRef> Section = *SectionOrErr)
    return link(*Section, RemarkFormat);
  return Error::success();
}

Error RemarkLinker::serialize(raw_ostream &OS, Format RemarksFormat) const {
  Expected<std::unique_ptr<RemarkSerializer>> MaybeSerializer =
      createRemarkSerializer(RemarksFormat, OS,
                             std::move(const_cast<StringTable &>(StrTab)));
  if (!MaybeSerializer)
    return MaybeSerializer.takeError();

  std::unique_ptr<remarks::RemarkSerializer> Serializer =
      std::move(*MaybeSerializer);

  for (const Remark &R : remarks())
    Serializer->emit(R);
  return Error::success();
}
