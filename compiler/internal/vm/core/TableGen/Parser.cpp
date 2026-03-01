//===- Parser.cpp - Top-Level TableGen Parser implementation --------------===//
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

#include "vm/core/TableGen/Parser.h"
#include "TGParser.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/VirtualFileSystem.h"
#include "vm/core/TableGen/Record.h"

using namespace vm::core;

bool toolchain::TableGenParseFile(SourceMgr &InputSrcMgr, RecordKeeper &Records) {
  // Initialize the global TableGen source manager by temporarily taking control
  // of the input buffer in `SrcMgr`. This is kind of a hack, but allows for
  // preserving TableGen's current awkward diagnostic behavior. If we can remove
  // this reliance, we could drop all of this.
  SrcMgr = SourceMgr();
  SrcMgr.takeSourceBuffersFrom(InputSrcMgr);
  SrcMgr.setIncludeDirs(InputSrcMgr.getIncludeDirs());
  SrcMgr.setVirtualFileSystem(InputSrcMgr.getVirtualFileSystem());
  SrcMgr.setDiagHandler(InputSrcMgr.getDiagHandler(),
                        InputSrcMgr.getDiagContext());

  // Setup the record keeper and try to parse the file.
  auto *MainFileBuffer = SrcMgr.getMemoryBuffer(SrcMgr.getMainFileID());
  Records.saveInputFilename(MainFileBuffer->getBufferIdentifier().str());

  TGParser Parser(SrcMgr, /*Macros=*/{}, Records,
                  /*NoWarnOnUnusedTemplateArgs=*/false,
                  /*TrackReferenceLocs=*/true);
  bool ParseResult = Parser.ParseFile();

  // After parsing, reclaim the source manager buffers from TableGen's global
  // manager.
  InputSrcMgr.takeSourceBuffersFrom(SrcMgr);
  SrcMgr = SourceMgr();
  return ParseResult;
}
