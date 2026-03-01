//===-- yaml2obj.cpp ------------------------------------------------------===//
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

#include "vm/core/ObjectYAML/yaml2obj.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/ObjectYAML/ObjectYAML.h"
#include "vm/core/Support/WithColor.h"
#include "vm/core/Support/YAMLTraits.h"

namespace vm::core {
namespace yaml {

bool convertYAML(yaml::Input &YIn, raw_ostream &Out, ErrorHandler ErrHandler,
                 unsigned DocNum, uint64_t MaxSize) {
  unsigned CurDocNum = 0;
  do {
    if (++CurDocNum != DocNum)
      continue;

    yaml::YamlObjectFile Doc;
    YIn >> Doc;
    if (std::error_code EC = YIn.error()) {
      ErrHandler("failed to parse YAML input: " + EC.message());
      return false;
    }

    if (Doc.Arch)
      return yaml2archive(*Doc.Arch, Out, ErrHandler);
    if (Doc.Elf)
      return yaml2elf(*Doc.Elf, Out, ErrHandler, MaxSize);
    if (Doc.Coff)
      return yaml2coff(*Doc.Coff, Out, ErrHandler);
    if (Doc.Goff)
      return yaml2goff(*Doc.Goff, Out, ErrHandler);
    if (Doc.MachO || Doc.FatMachO)
      return yaml2macho(Doc, Out, ErrHandler);
    if (Doc.Minidump)
      return yaml2minidump(*Doc.Minidump, Out, ErrHandler);
    if (Doc.Offload)
      return yaml2offload(*Doc.Offload, Out, ErrHandler);
    if (Doc.Wasm)
      return yaml2wasm(*Doc.Wasm, Out, ErrHandler);
    if (Doc.Xcoff)
      return yaml2xcoff(*Doc.Xcoff, Out, ErrHandler);
    if (Doc.DXContainer)
      return yaml2dxcontainer(*Doc.DXContainer, Out, ErrHandler);

    ErrHandler("unknown document type");
    return false;

  } while (YIn.nextDocument());

  ErrHandler("cannot find the " + Twine(DocNum) +
             getOrdinalSuffix(DocNum).data() + " document");
  return false;
}

std::unique_ptr<object::ObjectFile>
yaml2ObjectFile(SmallVectorImpl<char> &Storage, StringRef Yaml,
                ErrorHandler ErrHandler) {
  Storage.clear();
  raw_svector_ostream OS(Storage);

  yaml::Input YIn(Yaml);
  if (!convertYAML(YIn, OS, ErrHandler))
    return {};

  Expected<std::unique_ptr<object::ObjectFile>> ObjOrErr =
      object::ObjectFile::createObjectFile(
          MemoryBufferRef(OS.str(), "YamlObject"));
  if (ObjOrErr)
    return std::move(*ObjOrErr);

  ErrHandler(toString(ObjOrErr.takeError()));
  return {};
}

} // namespace yaml
} // namespace vm::core
