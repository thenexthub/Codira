//===- toolchain/BinaryFormat/COFF.cpp - The COFF format -----------------------===//
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

#include "vm/core/BinaryFormat/COFF.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Twine.h"

// Maximum offsets for different string table entry encodings.
enum : unsigned { Max7DecimalOffset = 9999999U };
enum : uint64_t { MaxBase64Offset = 0xFFFFFFFFFULL }; // 64^6, including 0

// Encode a string table entry offset in base 64, padded to 6 chars, and
// prefixed with a double slash: '//AAAAAA', '//AAAAAB', ...
// Buffer must be at least 8 bytes large. No terminating null appended.
static void encodeBase64StringEntry(char *Buffer, uint64_t Value) {
  assert(Value > Max7DecimalOffset && Value <= MaxBase64Offset &&
         "Illegal section name encoding for value");

  static const char Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789+/";

  Buffer[0] = '/';
  Buffer[1] = '/';

  char *Ptr = Buffer + 7;
  for (unsigned i = 0; i < 6; ++i) {
    unsigned Rem = Value % 64;
    Value /= 64;
    *(Ptr--) = Alphabet[Rem];
  }
}

bool toolchain::COFF::encodeSectionName(char *Out, uint64_t Offset) {
  if (Offset <= Max7DecimalOffset) {
    // Offsets of 7 digits or less are encoded in ASCII.
    SmallVector<char, COFF::NameSize> Buffer;
    Twine('/').concat(Twine(Offset)).toVector(Buffer);
    assert(Buffer.size() <= COFF::NameSize && Buffer.size() >= 2);
    std::memcpy(Out, Buffer.data(), Buffer.size());
    return true;
  }

  if (Offset <= MaxBase64Offset) {
    // Starting with 10,000,000, offsets are encoded as base64.
    encodeBase64StringEntry(Out, Offset);
    return true;
  }

  // The offset is too large to be encoded.
  return false;
}
