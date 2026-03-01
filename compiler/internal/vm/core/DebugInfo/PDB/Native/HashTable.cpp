//===- HashTable.cpp - PDB Hash Table -------------------------------------===//
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

#include "vm/core/DebugInfo/PDB/Native/HashTable.h"
#include "vm/core/DebugInfo/PDB/Native/RawError.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/MathExtras.h"
#include <cstdint>
#include <utility>

using namespace vm::core;
using namespace vm::core::pdb;

Error toolchain::pdb::readSparseBitVector(BinaryStreamReader &Stream,
                                     SparseBitVector<> &V) {
  uint32_t NumWords;
  if (auto EC = Stream.readInteger(NumWords))
    return joinErrors(
        std::move(EC),
        make_error<RawError>(raw_error_code::corrupt_file,
                             "Expected hash table number of words"));

  for (uint32_t I = 0; I != NumWords; ++I) {
    uint32_t Word;
    if (auto EC = Stream.readInteger(Word))
      return joinErrors(std::move(EC),
                        make_error<RawError>(raw_error_code::corrupt_file,
                                             "Expected hash table word"));
    for (unsigned Idx = 0; Idx < 32; ++Idx)
      if (Word & (1U << Idx))
        V.set((I * 32) + Idx);
  }
  return Error::success();
}

Error toolchain::pdb::writeSparseBitVector(BinaryStreamWriter &Writer,
                                      SparseBitVector<> &Vec) {
  constexpr int BitsPerWord = 8 * sizeof(uint32_t);

  int ReqBits = Vec.find_last() + 1;
  uint32_t ReqWords = alignTo(ReqBits, BitsPerWord) / BitsPerWord;
  if (auto EC = Writer.writeInteger(ReqWords))
    return joinErrors(
        std::move(EC),
        make_error<RawError>(raw_error_code::corrupt_file,
                             "Could not write linear map number of words"));

  uint32_t Idx = 0;
  for (uint32_t I = 0; I != ReqWords; ++I) {
    uint32_t Word = 0;
    for (uint32_t WordIdx = 0; WordIdx < 32; ++WordIdx, ++Idx) {
      if (Vec.test(Idx))
        Word |= (1 << WordIdx);
    }
    if (auto EC = Writer.writeInteger(Word))
      return joinErrors(std::move(EC), make_error<RawError>(
                                           raw_error_code::corrupt_file,
                                           "Could not write linear map word"));
  }
  return Error::success();
}
