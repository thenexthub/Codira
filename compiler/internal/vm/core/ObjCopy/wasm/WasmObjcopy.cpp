//===- WasmObjcopy.cpp ----------------------------------------------------===//
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

#include "vm/core/ObjCopy/wasm/WasmObjcopy.h"
#include "WasmObject.h"
#include "WasmReader.h"
#include "WasmWriter.h"
#include "vm/core/ObjCopy/CommonConfig.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/FileOutputBuffer.h"

namespace vm::core {
namespace objcopy {
namespace wasm {

using namespace object;
using SectionPred = std::function<bool(const Section &Sec)>;

static bool isDebugSection(const Section &Sec) {
  return Sec.Name.starts_with(".debug") || Sec.Name.starts_with("reloc..debug");
}

static bool isLinkerSection(const Section &Sec) {
  return Sec.Name.starts_with("reloc.") || Sec.Name == "linking";
}

static bool isNameSection(const Section &Sec) { return Sec.Name == "name"; }

// Sections which are known to be "comments" or informational and do not affect
// program semantics.
static bool isCommentSection(const Section &Sec) {
  return Sec.Name == "producers";
}

static Error dumpSectionToFile(StringRef SecName, StringRef Filename,
                               StringRef InputFilename, Object &Obj) {
  for (const Section &Sec : Obj.Sections) {
    if (Sec.Name == SecName) {
      ArrayRef<uint8_t> Contents = Sec.Contents;
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
  }
  return createFileError(Filename, errc::invalid_argument,
                         "section '%s' not found", SecName.str().c_str());
}

static void removeSections(const CommonConfig &Config, Object &Obj) {
  SectionPred RemovePred = [](const Section &) { return false; };

  // Explicitly-requested sections.
  if (!Config.ToRemove.empty()) {
    RemovePred = [&Config](const Section &Sec) {
      return Config.ToRemove.matches(Sec.Name);
    };
  }

  if (Config.StripDebug) {
    RemovePred = [RemovePred](const Section &Sec) {
      return RemovePred(Sec) || isDebugSection(Sec);
    };
  }

  if (Config.StripAll) {
    RemovePred = [RemovePred](const Section &Sec) {
      return RemovePred(Sec) || isDebugSection(Sec) || isLinkerSection(Sec) ||
             isNameSection(Sec) || isCommentSection(Sec);
    };
  }

  if (Config.OnlyKeepDebug) {
    RemovePred = [&Config](const Section &Sec) {
      // Keep debug sections, unless explicitly requested to remove.
      // Remove everything else, including known sections.
      return Config.ToRemove.matches(Sec.Name) || !isDebugSection(Sec);
    };
  }

  if (!Config.OnlySection.empty()) {
    RemovePred = [&Config](const Section &Sec) {
      // Explicitly keep these sections regardless of previous removes.
      // Remove everything else, inluding known sections.
      return !Config.OnlySection.matches(Sec.Name);
    };
  }

  if (!Config.KeepSection.empty()) {
    RemovePred = [&Config, RemovePred](const Section &Sec) {
      // Explicitly keep these sections regardless of previous removes.
      if (Config.KeepSection.matches(Sec.Name))
        return false;
      // Otherwise defer to RemovePred.
      return RemovePred(Sec);
    };
  }

  Obj.removeSections(RemovePred);
}

static Error handleArgs(const CommonConfig &Config, Object &Obj) {
  // Only support AddSection, DumpSection, RemoveSection for now.
  for (StringRef Flag : Config.DumpSection) {
    StringRef SecName;
    StringRef FileName;
    std::tie(SecName, FileName) = Flag.split("=");
    if (Error E =
            dumpSectionToFile(SecName, FileName, Config.InputFilename, Obj))
      return E;
  }

  removeSections(Config, Obj);

  for (const NewSectionInfo &NewSection : Config.AddSection) {
    Section Sec;
    Sec.SectionType = toolchain::wasm::WASM_SEC_CUSTOM;
    Sec.Name = NewSection.SectionName;

    toolchain::StringRef InputData =
        toolchain::StringRef(NewSection.SectionData->getBufferStart(),
                        NewSection.SectionData->getBufferSize());
    std::unique_ptr<MemoryBuffer> BufferCopy = MemoryBuffer::getMemBufferCopy(
        InputData, NewSection.SectionData->getBufferIdentifier());
    Sec.Contents = ArrayRef<uint8_t>(
        reinterpret_cast<const uint8_t *>(BufferCopy->getBufferStart()),
        BufferCopy->getBufferSize());

    Obj.addSectionWithOwnedContents(Sec, std::move(BufferCopy));
  }

  return Error::success();
}

Error executeObjcopyOnBinary(const CommonConfig &Config, const WasmConfig &,
                             object::WasmObjectFile &In, raw_ostream &Out) {
  Reader TheReader(In);
  Expected<std::unique_ptr<Object>> ObjOrErr = TheReader.create();
  if (!ObjOrErr)
    return createFileError(Config.InputFilename, ObjOrErr.takeError());
  Object *Obj = ObjOrErr->get();
  assert(Obj && "Unable to deserialize Wasm object");
  if (Error E = handleArgs(Config, *Obj))
    return E;
  Writer TheWriter(*Obj, Out);
  if (Error E = TheWriter.write())
    return createFileError(Config.OutputFilename, std::move(E));
  return Error::success();
}

} // end namespace wasm
} // end namespace objcopy
} // end namespace vm::core
