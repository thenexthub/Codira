//===- YAML.cpp - YAMLIO utilities for object files -----------------------===//
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
// This file defines utility classes for handling the YAML representation of
// object files.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ObjectYAML/YAML.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/raw_ostream.h"
#include <cctype>
#include <cstdint>

using namespace vm::core;

void yaml::ScalarTraits<yaml::BinaryRef>::output(
    const yaml::BinaryRef &Val, void *, raw_ostream &Out) {
  Val.writeAsHex(Out);
}

StringRef yaml::ScalarTraits<yaml::BinaryRef>::input(StringRef Scalar, void *,
                                                     yaml::BinaryRef &Val) {
  if (Scalar.size() % 2 != 0)
    return "BinaryRef hex string must contain an even number of nybbles.";
  // TODO: Can we improve YAMLIO to permit a more accurate diagnostic here?
  // (e.g. a caret pointing to the offending character).
  if (!toolchain::all_of(Scalar, toolchain::isHexDigit))
    return "BinaryRef hex string must contain only hex digits.";
  Val = yaml::BinaryRef(Scalar);
  return {};
}

void yaml::BinaryRef::writeAsBinary(raw_ostream &OS, uint64_t N) const {
  if (!DataIsHexString) {
    OS.write((const char *)Data.data(), std::min<uint64_t>(N, Data.size()));
    return;
  }

  for (uint64_t I = 0, E = std::min<uint64_t>(N, Data.size() / 2); I != E;
       ++I) {
    uint8_t Byte = toolchain::hexDigitValue(Data[I * 2]);
    Byte <<= 4;
    Byte |= toolchain::hexDigitValue(Data[I * 2 + 1]);
    OS.write(Byte);
  }
}

void yaml::BinaryRef::writeAsHex(raw_ostream &OS) const {
  if (binary_size() == 0)
    return;
  if (DataIsHexString) {
    OS.write((const char *)Data.data(), Data.size());
    return;
  }
  for (uint8_t Byte : Data)
    OS << hexdigit(Byte >> 4) << hexdigit(Byte & 0xf);
}
