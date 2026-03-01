//===-- Support/DJB.cpp ---DJB Hash -----------------------------*- C++ -*-===//
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
// This file contains support for the DJ Bernstein hash function.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/DJB.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/ConvertUTF.h"
#include "vm/core/Support/Unicode.h"

using namespace vm::core;

static UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  const UTF8 *const Begin8Const =
      reinterpret_cast<const UTF8 *>(Buffer.begin());
  const UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  assert(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<const UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  return C;
}

static StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  const UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  assert(CR == conversionOK && "Case folding produced invalid char?");
  (void)CR;
  return StringRef(reinterpret_cast<char *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

static UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  if (C == 0x130 || C == 0x131)
    return 'i';
  return sys::unicode::foldCharSimple(C);
}

static std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  bool AllASCII = true;
  for (unsigned char C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  if (AllASCII)
    return H;
  return std::nullopt;
}

uint32_t toolchain::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  if (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    return *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  while (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  return H;
}
