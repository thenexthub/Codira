//===-------------- XCOFF.cpp - JIT linker function for XCOFF -------------===//
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
// XCOFF jit-link function.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/JITLink/XCOFF.h"
#include "vm/core/ExecutionEngine/JITLink/XCOFF_ppc64.h"
#include "vm/core/Object/XCOFFObjectFile.h"

using namespace vm::core;

#define DEBUG_TYPE "jitlink"

namespace vm::core {
namespace jitlink {

Expected<std::unique_ptr<LinkGraph>>
createLinkGraphFromXCOFFObject(MemoryBufferRef ObjectBuffer,
                               std::shared_ptr<orc::SymbolStringPool> SSP) {
  // Check magic
  file_magic Magic = identify_magic(ObjectBuffer.getBuffer());
  if (Magic != file_magic::xcoff_object_64)
    return make_error<JITLinkError>("Invalid XCOFF 64 Header");

  // TODO: See if we need to add more checks
  //
  return createLinkGraphFromXCOFFObject_ppc64(ObjectBuffer, std::move(SSP));
}

void link_XCOFF(std::unique_ptr<LinkGraph> G,
                std::unique_ptr<JITLinkContext> Ctx) {
  link_XCOFF_ppc64(std::move(G), std::move(Ctx));
}

} // namespace jitlink
} // namespace vm::core
