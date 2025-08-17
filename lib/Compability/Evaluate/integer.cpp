//===-- lib/Evaluate/integer.cpp ------------------------------------------===//
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

#include "language/Compability/Evaluate/integer.h"

namespace language::Compability::evaluate::value {

template class Integer<8>;
template class Integer<16>;
template class Integer<32>;
template class Integer<64>;
template class Integer<80, isHostLittleEndian, 16, std::uint16_t, std::uint32_t,
    128>;
template class Integer<128>;

// Sanity checks against misconfiguration bugs
static_assert(Integer<8>::partBits == 8);
static_assert(std::is_same_v<typename Integer<8>::Part, std::uint8_t>);
static_assert(Integer<16>::partBits == 16);
static_assert(std::is_same_v<typename Integer<16>::Part, std::uint16_t>);
static_assert(Integer<32>::partBits == 32);
static_assert(std::is_same_v<typename Integer<32>::Part, std::uint32_t>);
static_assert(Integer<64>::partBits == 32);
static_assert(std::is_same_v<typename Integer<64>::Part, std::uint32_t>);
static_assert(Integer<128>::partBits == 32);
static_assert(std::is_same_v<typename Integer<128>::Part, std::uint32_t>);
} // namespace language::Compability::evaluate::value
