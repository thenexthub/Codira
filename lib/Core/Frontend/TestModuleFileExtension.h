//===-- TestModuleFileExtension.h - Module Extension Tester -----*- C++ -*-===//
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
#ifndef LANGUAGE_CORE_FRONTEND_TESTMODULEFILEEXTENSION_H
#define LANGUAGE_CORE_FRONTEND_TESTMODULEFILEEXTENSION_H

#include "language/Core/Serialization/ModuleFileExtension.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Bitstream/BitstreamReader.h"
#include <string>

namespace language::Core {

/// A module file extension used for testing purposes.
class TestModuleFileExtension
    : public toolchain::RTTIExtends<TestModuleFileExtension, ModuleFileExtension> {
  std::string BlockName;
  unsigned MajorVersion;
  unsigned MinorVersion;
  bool Hashed;
  std::string UserInfo;

  class Writer : public ModuleFileExtensionWriter {
  public:
    Writer(ModuleFileExtension *Ext) : ModuleFileExtensionWriter(Ext) { }
    ~Writer() override;

    void writeExtensionContents(Sema &SemaRef,
                                toolchain::BitstreamWriter &Stream) override;
  };

  class Reader : public ModuleFileExtensionReader {
    toolchain::BitstreamCursor Stream;

  public:
    ~Reader() override;

    Reader(ModuleFileExtension *Ext, const toolchain::BitstreamCursor &InStream);
  };

public:
  static char ID;

  TestModuleFileExtension(StringRef BlockName, unsigned MajorVersion,
                          unsigned MinorVersion, bool Hashed,
                          StringRef UserInfo)
      : BlockName(BlockName), MajorVersion(MajorVersion),
        MinorVersion(MinorVersion), Hashed(Hashed), UserInfo(UserInfo) {}
  ~TestModuleFileExtension() override;

  ModuleFileExtensionMetadata getExtensionMetadata() const override;

  void hashExtension(ExtensionHashBuilder &HBuilder) const override;

  std::unique_ptr<ModuleFileExtensionWriter>
  createExtensionWriter(ASTWriter &Writer) override;

  std::unique_ptr<ModuleFileExtensionReader>
  createExtensionReader(const ModuleFileExtensionMetadata &Metadata,
                        ASTReader &Reader, serialization::ModuleFile &Mod,
                        const toolchain::BitstreamCursor &Stream) override;

  std::string str() const;
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_FRONTEND_TESTMODULEFILEEXTENSION_H
