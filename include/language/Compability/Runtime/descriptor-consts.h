//===-- language/Compability/Runtime/descriptor-consts.h ---------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_DESCRIPTOR_CONSTS_H_
#define LANGUAGE_COMPABILITY_RUNTIME_DESCRIPTOR_CONSTS_H_

#include "language/Compability/Common/Fortran-consts.h"
#include "language/Compability/Common/ISO_Fortran_binding_wrapper.h"
#include "language/Compability/Common/api-attrs.h"
#include <cstddef>
#include <cstdint>

// Value of the addendum presence flag.
#define _CFI_ADDENDUM_FLAG 1
// Number of bits needed to be shifted when manipulating the allocator index.
#define _CFI_ALLOCATOR_IDX_SHIFT 1
// Allocator index mask.
#define _CFI_ALLOCATOR_IDX_MASK 0b00001110

namespace language::Compability::runtime::typeInfo {
using TypeParameterValue = std::int64_t;
class DerivedType;
} // namespace language::Compability::runtime::typeInfo

namespace language::Compability::runtime {
class Descriptor;
using SubscriptValue = ISO::CFI_index_t;
using common::TypeCategory;

/// Returns size in bytes of the descriptor (not the data)
/// This must be at least as large as the largest descriptor of any target
/// triple.
static constexpr RT_API_ATTRS std::size_t MaxDescriptorSizeInBytes(
    int rank, bool addendum = false, int lengthTypeParameters = 0) {
  // Layout:
  //
  // fortran::runtime::Descriptor {
  //   ISO::CFI_cdesc_t {
  //     void *base_addr;           (pointer -> up to 8 bytes)
  //     size_t elem_len;           (up to 8 bytes)
  //     int version;               (up to 4 bytes)
  //     CFI_rank_t rank;           (unsigned char -> 1 byte)
  //     CFI_type_t type;           (signed char -> 1 byte)
  //     CFI_attribute_t attribute; (unsigned char -> 1 byte)
  //     unsigned char extra;       (1 byte)
  //   }
  // }
  // fortran::runtime::Dimension[rank] {
  //   ISO::CFI_dim_t {
  //     CFI_index_t lower_bound; (ptrdiff_t -> up to 8 bytes)
  //     CFI_index_t extent;      (ptrdiff_t -> up to 8 bytes)
  //     CFI_index_t sm;          (ptrdiff_t -> up to 8 bytes)
  //   }
  // }
  // fortran::runtime::DescriptorAddendum {
  //   const typeInfo::DerivedType *derivedType_;        (pointer -> up to 8
  //   bytes) typeInfo::TypeParameterValue len_[lenParameters]; (int64_t -> 8
  //   bytes)
  // }
  std::size_t bytes{24u + rank * 24u};
  if (addendum || lengthTypeParameters > 0) {
    if (lengthTypeParameters < 1)
      lengthTypeParameters = 1;
    bytes += 8u + static_cast<std::size_t>(lengthTypeParameters) * 8u;
  }
  return bytes;
}

} // namespace language::Compability::runtime

#endif /* FORTRAN_RUNTIME_DESCRIPTOR_CONSTS_H_ */
