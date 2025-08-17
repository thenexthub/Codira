//===- NativeFormatting.h - Low level formatting helpers ---------*- C++-*-===//
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

#ifndef LLVM_SUPPORT_NATIVE_FORMATTING_H
#define LLVM_SUPPORT_NATIVE_FORMATTING_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_Optional.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_raw_ostream.h>

#include <cstdint>

namespace toolchain {
enum class FloatStyle { Exponent, ExponentUpper, Fixed, Percent };
enum class IntegerStyle {
  Integer,
  Number,
};
enum class HexPrintStyle { Upper, Lower, PrefixUpper, PrefixLower };

size_t getDefaultPrecision(FloatStyle Style);

bool isPrefixedHexStyle(HexPrintStyle S);

void write_integer(raw_ostream &S, unsigned int N, size_t MinDigits,
                   IntegerStyle Style);
void write_integer(raw_ostream &S, int N, size_t MinDigits, IntegerStyle Style);
void write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                   IntegerStyle Style);
void write_integer(raw_ostream &S, long N, size_t MinDigits,
                   IntegerStyle Style);
void write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                   IntegerStyle Style);
void write_integer(raw_ostream &S, long long N, size_t MinDigits,
                   IntegerStyle Style);

void write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
               Optional<size_t> Width = None);
void write_double(raw_ostream &S, double D, FloatStyle Style,
                  Optional<size_t> Precision = None);
}

#endif

