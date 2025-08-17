//===--- ScratchBuffer.h - Scratch space for forming tokens -----*- C++ -*-===//
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
//
//  This file defines the ScratchBuffer interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LEX_SCRATCHBUFFER_H
#define LANGUAGE_CORE_LEX_SCRATCHBUFFER_H

#include "language/Core/Basic/SourceLocation.h"

namespace language::Core {
  class SourceManager;

/// ScratchBuffer - This class exposes a simple interface for the dynamic
/// construction of tokens.  This is used for builtin macros (e.g. __LINE__) as
/// well as token pasting, etc.
class ScratchBuffer {
  SourceManager &SourceMgr;
  char *CurBuffer;
  SourceLocation BufferStartLoc;
  unsigned BytesUsed;
public:
  ScratchBuffer(SourceManager &SM);

  /// getToken - Splat the specified text into a temporary MemoryBuffer and
  /// return a SourceLocation that refers to the token.  This is just like the
  /// previous method, but returns a location that indicates the physloc of the
  /// token.
  SourceLocation getToken(const char *Buf, unsigned Len, const char *&DestPtr);

private:
  void AllocScratchBuffer(unsigned RequestLen);
};

} // end namespace language::Core

#endif
