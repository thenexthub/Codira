//===- InfoStreamBuilder.cpp - PDB Info Stream Creation ---------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/InfoStreamBuilder.h"

#include "vm/core/DebugInfo/MSF/MSFBuilder.h"
#include "vm/core/DebugInfo/MSF/MappedBlockStream.h"
#include "vm/core/DebugInfo/PDB/Native/NamedStreamMap.h"
#include "vm/core/DebugInfo/PDB/Native/RawTypes.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/TimeProfiler.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::msf;
using namespace vm::core::pdb;

InfoStreamBuilder::InfoStreamBuilder(msf::MSFBuilder &Msf,
                                     NamedStreamMap &NamedStreams)
    : Msf(Msf), Ver(PdbRaw_ImplVer::PdbImplVC70), Age(0),
      NamedStreams(NamedStreams) {
  ::memset(&Guid, 0, sizeof(Guid));
}

void InfoStreamBuilder::setVersion(PdbRaw_ImplVer V) { Ver = V; }

void InfoStreamBuilder::addFeature(PdbRaw_FeatureSig Sig) {
  Features.push_back(Sig);
}

void InfoStreamBuilder::setHashPDBContentsToGUID(bool B) {
  HashPDBContentsToGUID = B;
}

void InfoStreamBuilder::setAge(uint32_t A) { Age = A; }

void InfoStreamBuilder::setSignature(uint32_t S) { Signature = S; }

void InfoStreamBuilder::setGuid(GUID G) { Guid = G; }


Error InfoStreamBuilder::finalizeMsfLayout() {
  uint32_t Length = sizeof(InfoStreamHeader) +
                    NamedStreams.calculateSerializedLength() +
                    (Features.size() + 1) * sizeof(uint32_t);
  if (auto EC = Msf.setStreamSize(StreamPDB, Length))
    return EC;
  return Error::success();
}

Error InfoStreamBuilder::commit(const msf::MSFLayout &Layout,
                                WritableBinaryStreamRef Buffer) const {
  toolchain::TimeTraceScope timeScope("Commit info stream");
  auto InfoS = WritableMappedBlockStream::createIndexedStream(
      Layout, Buffer, StreamPDB, Msf.getAllocator());
  BinaryStreamWriter Writer(*InfoS);

  InfoStreamHeader H;
  // Leave the build id fields 0 so they can be set as the last step before
  // committing the file to disk.
  ::memset(&H, 0, sizeof(H));
  H.Version = Ver;
  if (auto EC = Writer.writeObject(H))
    return EC;

  if (auto EC = NamedStreams.commit(Writer))
    return EC;
  if (auto EC = Writer.writeInteger(0))
    return EC;
  for (auto E : Features) {
    if (auto EC = Writer.writeEnum(E))
      return EC;
  }
  assert(Writer.bytesRemaining() == 0);
  return Error::success();
}
