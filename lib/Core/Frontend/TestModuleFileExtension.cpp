//===-- TestModuleFileExtension.cpp - Module Extension Tester -------------===//
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
#include "TestModuleFileExtension.h"
#include "language/Core/Frontend/FrontendDiagnostic.h"
#include "language/Core/Serialization/ASTReader.h"
#include "toolchain/Bitstream/BitstreamWriter.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdio>
using namespace language::Core;
using namespace language::Core::serialization;

char TestModuleFileExtension::ID = 0;

TestModuleFileExtension::Writer::~Writer() { }

void TestModuleFileExtension::Writer::writeExtensionContents(
       Sema &SemaRef,
       toolchain::BitstreamWriter &Stream) {
  using namespace toolchain;

  // Write an abbreviation for this record.
  auto Abv = std::make_shared<toolchain::BitCodeAbbrev>();
  Abv->Add(BitCodeAbbrevOp(FIRST_EXTENSION_RECORD_ID));
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // # of characters
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));   // message
  auto Abbrev = Stream.EmitAbbrev(std::move(Abv));

  // Write a message into the extension block.
  SmallString<64> Message;
  {
    auto Ext = static_cast<TestModuleFileExtension *>(getExtension());
    raw_svector_ostream OS(Message);
    OS << "Hello from " << Ext->BlockName << " v" << Ext->MajorVersion << "."
       << Ext->MinorVersion;
  }
  uint64_t Record[] = {FIRST_EXTENSION_RECORD_ID, Message.size()};
  Stream.EmitRecordWithBlob(Abbrev, Record, Message);
}

TestModuleFileExtension::Reader::Reader(ModuleFileExtension *Ext,
                                        const toolchain::BitstreamCursor &InStream)
  : ModuleFileExtensionReader(Ext), Stream(InStream)
{
  // Read the extension block.
  SmallVector<uint64_t, 4> Record;
  while (true) {
    toolchain::Expected<toolchain::BitstreamEntry> MaybeEntry =
        Stream.advanceSkippingSubblocks();
    if (!MaybeEntry)
      (void)MaybeEntry.takeError();
    toolchain::BitstreamEntry Entry = MaybeEntry.get();

    switch (Entry.Kind) {
    case toolchain::BitstreamEntry::SubBlock:
    case toolchain::BitstreamEntry::EndBlock:
    case toolchain::BitstreamEntry::Error:
      return;

    case toolchain::BitstreamEntry::Record:
      break;
    }

    Record.clear();
    StringRef Blob;
    Expected<unsigned> MaybeRecCode =
        Stream.readRecord(Entry.ID, Record, &Blob);
    if (!MaybeRecCode)
      fprintf(stderr, "Failed reading rec code: %s\n",
              toString(MaybeRecCode.takeError()).c_str());
    switch (MaybeRecCode.get()) {
    case FIRST_EXTENSION_RECORD_ID: {
      StringRef Message = Blob.substr(0, Record[0]);
      fprintf(stderr, "Read extension block message: %s\n",
              Message.str().c_str());
      break;
    }
    }
  }
}

TestModuleFileExtension::Reader::~Reader() { }

TestModuleFileExtension::~TestModuleFileExtension() { }

ModuleFileExtensionMetadata
TestModuleFileExtension::getExtensionMetadata() const {
  return { BlockName, MajorVersion, MinorVersion, UserInfo };
}

void TestModuleFileExtension::hashExtension(
    ExtensionHashBuilder &HBuilder) const {
  if (Hashed) {
    HBuilder.add(BlockName);
    HBuilder.add(MajorVersion);
    HBuilder.add(MinorVersion);
    HBuilder.add(UserInfo);
  }
}

std::unique_ptr<ModuleFileExtensionWriter>
TestModuleFileExtension::createExtensionWriter(ASTWriter &) {
  return std::unique_ptr<ModuleFileExtensionWriter>(new Writer(this));
}

std::unique_ptr<ModuleFileExtensionReader>
TestModuleFileExtension::createExtensionReader(
  const ModuleFileExtensionMetadata &Metadata,
  ASTReader &Reader, serialization::ModuleFile &Mod,
  const toolchain::BitstreamCursor &Stream)
{
  assert(Metadata.BlockName == BlockName && "Wrong block name");
  if (std::make_pair(Metadata.MajorVersion, Metadata.MinorVersion) !=
        std::make_pair(MajorVersion, MinorVersion)) {
    Reader.getDiags().Report(Mod.ImportLoc,
                             diag::err_test_module_file_extension_version)
      << BlockName << Metadata.MajorVersion << Metadata.MinorVersion
      << MajorVersion << MinorVersion;
    return nullptr;
  }

  return std::unique_ptr<ModuleFileExtensionReader>(
                                                    new TestModuleFileExtension::Reader(this, Stream));
}

std::string TestModuleFileExtension::str() const {
  std::string Buffer;
  toolchain::raw_string_ostream OS(Buffer);
  OS << BlockName << ":" << MajorVersion << ":" << MinorVersion << ":" << Hashed
     << ":" << UserInfo;
  return Buffer;
}
