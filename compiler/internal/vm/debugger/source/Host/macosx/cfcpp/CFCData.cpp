//===-- CFCData.cpp -------------------------------------------------------===//
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

#include "CFCData.h"

// CFCData constructor
CFCData::CFCData(CFDataRef data) : CFCReleaser<CFDataRef>(data) {}

// CFCData copy constructor
CFCData::CFCData(const CFCData &rhs) = default;

// CFCData copy constructor
CFCData &CFCData::operator=(const CFCData &rhs)

{
  if (this != &rhs)
    *this = rhs;
  return *this;
}

// Destructor
CFCData::~CFCData() = default;

CFIndex CFCData::GetLength() const {
  CFDataRef data = get();
  if (data)
    return CFDataGetLength(data);
  return 0;
}

const uint8_t *CFCData::GetBytePtr() const {
  CFDataRef data = get();
  if (data)
    return CFDataGetBytePtr(data);
  return NULL;
}

CFDataRef CFCData::Serialize(CFPropertyListRef plist,
                             CFPropertyListFormat format) {
  CFAllocatorRef alloc = kCFAllocatorDefault;
  reset();
  CFCReleaser<CFWriteStreamRef> stream(
      ::CFWriteStreamCreateWithAllocatedBuffers(alloc, alloc));
  ::CFWriteStreamOpen(stream.get());
  CFIndex len = ::CFPropertyListWrite(plist, stream.get(), format, 0, nullptr);
  if (len > 0)
    reset((CFDataRef)::CFWriteStreamCopyProperty(stream.get(),
                                                 kCFStreamPropertyDataWritten));
  ::CFWriteStreamClose(stream.get());
  return get();
}
