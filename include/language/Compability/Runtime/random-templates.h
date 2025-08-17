//===-- language/Compability-rt/runtime/random-templates.h -------------*- C++ -*-===//
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

#ifndef FLANG_RT_RUNTIME_RANDOM_TEMPLATES_H_
#define FLANG_RT_RUNTIME_RANDOM_TEMPLATES_H_

#include "descriptor.h"
#include "lock.h"
#include "numeric-templates.h"
#include "flang/Common/optional.h"
#include <algorithm>
#include <random>

namespace language::Compability::runtime::random {

// Newer "Minimum standard", recommended by Park, Miller, and Stockmeyer in
// 1993. Same as C++17 std::minstd_rand, but explicitly instantiated for
// permanence.
using Generator =
    std::linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647>;

using GeneratedWord = typename Generator::result_type;
static constexpr std::uint64_t range{
    static_cast<std::uint64_t>(Generator::max() - Generator::min() + 1)};
static constexpr bool rangeIsPowerOfTwo{(range & (range - 1)) == 0};
static constexpr int rangeBits{
    64 - common::LeadingZeroBitCount(range) - !rangeIsPowerOfTwo};

extern Lock lock;
extern Generator generator;
extern language::Compability::common::optional<GeneratedWord> nextValue;

// Call only with lock held
static GeneratedWord GetNextValue() {
  GeneratedWord result;
  if (nextValue.has_value()) {
    result = *nextValue;
    nextValue.reset();
  } else {
    result = generator();
  }
  return result;
}

template <typename REAL, int PREC>
inline void GenerateReal(const Descriptor &harvest) {
  static constexpr std::size_t minBits{
      std::max<std::size_t>(PREC, 8 * sizeof(GeneratedWord))};
  using Int = common::HostUnsignedIntType<minBits>;
  static constexpr std::size_t words{
      static_cast<std::size_t>(PREC + rangeBits - 1) / rangeBits};
  std::size_t elements{harvest.Elements()};
  SubscriptValue at[maxRank];
  harvest.GetLowerBounds(at);
  {
    CriticalSection critical{lock};
    for (std::size_t j{0}; j < elements; ++j) {
      while (true) {
        Int fraction{GetNextValue()};
        if constexpr (words > 1) {
          for (std::size_t k{1}; k < words; ++k) {
            static constexpr auto rangeMask{
                (GeneratedWord{1} << rangeBits) - 1};
            GeneratedWord word{(GetNextValue() - generator.min()) & rangeMask};
            fraction = (fraction << rangeBits) | word;
          }
        }
        fraction >>= words * rangeBits - PREC;
        REAL next{
            LDEXPTy<REAL>::compute(static_cast<REAL>(fraction), -(PREC + 1))};
        if (next >= 0.0 && next < 1.0) {
          *harvest.Element<REAL>(at) = next;
          break;
        }
      }
      harvest.IncrementSubscripts(at);
    }
  }
}

template <typename UINT>
inline void GenerateUnsigned(const Descriptor &harvest) {
  static constexpr std::size_t words{
      (8 * sizeof(UINT) + rangeBits - 1) / rangeBits};
  std::size_t elements{harvest.Elements()};
  SubscriptValue at[maxRank];
  harvest.GetLowerBounds(at);
  {
    CriticalSection critical{lock};
    for (std::size_t j{0}; j < elements; ++j) {
      UINT next{static_cast<UINT>(GetNextValue())};
      if constexpr (words > 1) {
        for (std::size_t k{1}; k < words; ++k) {
          next <<= rangeBits;
          next |= GetNextValue();
        }
      }
      *harvest.Element<UINT>(at) = next;
      harvest.IncrementSubscripts(at);
    }
  }
}

} // namespace language::Compability::runtime::random

#endif // FLANG_RT_RUNTIME_RANDOM_TEMPLATES_H_
