//===- DXContainerObjcopy.cpp ---------------------------------------------===//
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

#include "vm/core/ObjCopy/DXContainer/DXContainerObjcopy.h"
#include "DXContainerReader.h"
#include "DXContainerWriter.h"
#include "vm/core/BinaryFormat/DXContainer.h"
#include "vm/core/ObjCopy/CommonConfig.h"
#include "vm/core/ObjCopy/DXContainer/DXContainerConfig.h"
#include "vm/core/Support/FileOutputBuffer.h"
#include "vm/core/Support/raw_ostream.h"

namespace vm::core {
namespace objcopy {
namespace dxbc {

using namespace object;

static Error extractPartAsObject(StringRef PartName, StringRef OutFilename,
                                 StringRef InputFilename, const Object &Obj) {
  for (const Part &P : Obj.Parts)
    if (P.Name == PartName) {
      Object PartObj;
      PartObj.Header = Obj.Header;
      PartObj.Parts.push_back({P.Name, P.Data});
      PartObj.recomputeHeader();

      auto Write = [&OutFilename, &PartObj](raw_ostream &Out) -> Error {
        DXContainerWriter Writer(PartObj, Out);
        if (Error E = Writer.write())
          return createFileError(OutFilename, std::move(E));
        return Error::success();
      };

      return writeToOutput(OutFilename, Write);
    }

  return createFileError(InputFilename, object_error::parse_failed,
                         "part '%s' not found", PartName.str().c_str());
}

static Error dumpPartToFile(StringRef PartName, StringRef Filename,
                            StringRef InputFilename, Object &Obj) {
  auto PartIter = toolchain::find_if(
      Obj.Parts, [&PartName](const Part &P) { return P.Name == PartName; });
  if (PartIter == Obj.Parts.end())
    return createFileError(Filename,
                           std::make_error_code(std::errc::invalid_argument),
                           "part '%s' not found", PartName.str().c_str());
  ArrayRef<uint8_t> Contents = PartIter->Data;
  // The DXContainer format is a bit odd because the part-specific headers are
  // contained inside the part data itself. For parts that contain LLVM bitcode
  // when we dump the part we want to skip the part-specific header so that we
  // get a valid .bc file that we can inspect. All the data contained inside the
  // program header is pulled out of the bitcode, so the header can be
  // reconstructed if needed from the bitcode itself. More comprehensive
  // documentation on the DXContainer format can be found at
  // https://toolchain.org/docs/DirectX/DXContainer.html.

  if (PartName == "DXIL" || PartName == "STAT")
    Contents = Contents.drop_front(sizeof(toolchain::dxbc::ProgramHeader));
  if (Contents.empty())
    return createFileError(Filename, object_error::parse_failed,
                           "part '%s' is empty", PartName.str().c_str());
  Expected<std::unique_ptr<FileOutputBuffer>> BufferOrErr =
      FileOutputBuffer::create(Filename, Contents.size());
  if (!BufferOrErr)
    return createFileError(Filename, BufferOrErr.takeError());
  std::unique_ptr<FileOutputBuffer> Buf = std::move(*BufferOrErr);
  toolchain::copy(Contents, Buf->getBufferStart());
  if (Error E = Buf->commit())
    return createFileError(Filename, std::move(E));
  return Error::success();
}

static Error handleArgs(const CommonConfig &Config, Object &Obj) {
  for (StringRef Flag : Config.DumpSection) {
    auto [SecName, FileName] = Flag.split("=");
    if (Error E = dumpPartToFile(SecName, FileName, Config.InputFilename, Obj))
      return E;
  }

  // Extract all sections before any modifications.
  for (StringRef Flag : Config.ExtractSection) {
    StringRef SectionName;
    StringRef FileName;
    std::tie(SectionName, FileName) = Flag.split('=');
    if (Error E = extractPartAsObject(SectionName, FileName,
                                      Config.InputFilename, Obj))
      return E;
  }

  std::function<bool(const Part &)> RemovePred = [](const Part &) {
    return false;
  };

  if (!Config.ToRemove.empty())
    RemovePred = [&Config](const Part &P) {
      return Config.ToRemove.matches(P.Name);
    };

  if (!Config.OnlySection.empty())
    RemovePred = [&Config](const Part &P) {
      // Explicitly keep these sections regardless of previous removes and
      // remove everything else.
      return !Config.OnlySection.matches(P.Name);
    };

  if (auto E = Obj.removeParts(RemovePred))
    return E;

  Obj.recomputeHeader();
  return Error::success();
}

Error executeObjcopyOnBinary(const CommonConfig &Config,
                             const DXContainerConfig &,
                             DXContainerObjectFile &In, raw_ostream &Out) {
  DXContainerReader Reader(In);
  Expected<std::unique_ptr<Object>> ObjOrErr = Reader.create();
  if (!ObjOrErr)
    return createFileError(Config.InputFilename, ObjOrErr.takeError());
  Object *Obj = ObjOrErr->get();
  assert(Obj && "Unable to deserialize DXContainer object");

  if (Error E = handleArgs(Config, *Obj))
    return E;

  DXContainerWriter Writer(*Obj, Out);
  if (Error E = Writer.write())
    return createFileError(Config.OutputFilename, std::move(E));
  return Error::success();
}

} // end namespace dxbc
} // end namespace objcopy
} // end namespace vm::core
