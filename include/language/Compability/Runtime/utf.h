//===-- language/Compability-rt/runtime/utf.h --------------------------*- C++ -*-===//
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

// UTF-8 is the variant-width standard encoding of Unicode (ISO 10646)
// code points.
//
// 7-bit values in [00 .. 7F] represent themselves as single bytes, so true
// 7-bit ASCII is also valid UTF-8.
//
// Larger values are encoded with a start byte in [C0 .. FE] that carries
// the length of the encoding and some of the upper bits of the value, followed
// by one or more bytes in the range [80 .. BF].
//
// Specifically, the first byte holds two or more uppermost set bits,
// a zero bit, and some payload; the second and later bytes each start with
// their uppermost bit set, the next bit clear, and six bits of payload.
// Payload parcels are in big-endian order.  All bytes must be present in a
// valid sequence; i.e., low-order sezo bits must be explicit.  UTF-8 is
// self-synchronizing on input as any byte value cannot be both a valid
// first byte or trailing byte.
//
// 0xxxxxxx - 7 bit ASCII
// 110xxxxx 10xxxxxx - 11-bit value
// 1110xxxx 10xxxxxx 10xxxxxx - 16-bit value
// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx - 21-bit value
// 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx - 26-bit value
// 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx - 31-bit value
// 11111110 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx - 36-bit value
//
// Canonical UTF-8 sequences should be minimal, and our output is so, but
// we do not reject non-minimal sequences on input.  Unicode only defines
// code points up to 0x10FFFF, so 21-bit (4-byte) UTF-8 is the actual
// standard maximum.  However, we support extended forms up to 32 bits so that
// CHARACTER(KIND=4) can be abused to hold arbitrary 32-bit data.

#ifndef FLANG_RT_RUNTIME_UTF_H_
#define FLANG_RT_RUNTIME_UTF_H_

#include "flang/Common/optional.h"
#include <cstddef>
#include <cstdint>

namespace language::Compability::runtime {

// Derive the length of a UTF-8 character encoding from its first byte.
// A zero result signifies an invalid encoding.
RT_OFFLOAD_VAR_GROUP_BEGIN
extern const RT_CONST_VAR_ATTRS std::uint8_t UTF8FirstByteTable[256];
static constexpr std::size_t maxUTF8Bytes{7};
RT_OFFLOAD_VAR_GROUP_END

static inline RT_API_ATTRS std::size_t MeasureUTF8Bytes(char first) {
  return UTF8FirstByteTable[static_cast<std::uint8_t>(first)];
}

RT_API_ATTRS std::size_t MeasurePreviousUTF8Bytes(
    const char *end, std::size_t limit);

// Ensure that all bytes are present in sequence in the input buffer
// before calling; use MeasureUTF8Bytes(first byte) to count them.
RT_API_ATTRS language::Compability::common::optional<char32_t> DecodeUTF8(const char *);

// Ensure that at least maxUTF8Bytes remain in the output
// buffer before calling.
RT_API_ATTRS std::size_t EncodeUTF8(char *, char32_t);

} // namespace language::Compability::runtime
#endif // FLANG_RT_RUNTIME_UTF_H_
