//===-- language/Compability-rt/runtime/internal-unit.h ----------------*- C++ -*-===//
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

// Fortran internal I/O "units"

#ifndef FLANG_RT_RUNTIME_INTERNAL_UNIT_H_
#define FLANG_RT_RUNTIME_INTERNAL_UNIT_H_

#include "connection.h"
#include "descriptor.h"
#include <cinttypes>
#include <type_traits>

namespace language::Compability::runtime::io {

RT_OFFLOAD_API_GROUP_BEGIN

class IoErrorHandler;

// Points to (but does not own) a CHARACTER scalar or array for internal I/O.
// Does not buffer.
template <Direction DIR> class InternalDescriptorUnit : public ConnectionState {
public:
  using Scalar =
      std::conditional_t<DIR == Direction::Input, const char *, char *>;
  RT_API_ATTRS InternalDescriptorUnit(Scalar, std::size_t chars, int kind);
  RT_API_ATTRS InternalDescriptorUnit(const Descriptor &, const Terminator &);

  RT_API_ATTRS bool Emit(const char *, std::size_t, IoErrorHandler &);
  RT_API_ATTRS std::size_t GetNextInputBytes(const char *&, IoErrorHandler &);
  RT_API_ATTRS std::size_t ViewBytesInRecord(const char *&, bool forward) const;
  RT_API_ATTRS bool AdvanceRecord(IoErrorHandler &);
  RT_API_ATTRS void BackspaceRecord(IoErrorHandler &);
  RT_API_ATTRS std::int64_t InquirePos();

private:
  RT_API_ATTRS Descriptor &descriptor() {
    return staticDescriptor_.descriptor();
  }
  RT_API_ATTRS const Descriptor &descriptor() const {
    return staticDescriptor_.descriptor();
  }
  RT_API_ATTRS Scalar CurrentRecord() const {
    return descriptor().template ZeroBasedIndexedElement<char>(
        currentRecordNumber - 1);
  }
  RT_API_ATTRS void BlankFill(char *, std::size_t);
  RT_API_ATTRS void BlankFillOutputRecord();

  StaticDescriptor<maxRank, true /*addendum*/> staticDescriptor_;
};

extern template class InternalDescriptorUnit<Direction::Output>;
extern template class InternalDescriptorUnit<Direction::Input>;

RT_OFFLOAD_API_GROUP_END

} // namespace language::Compability::runtime::io
#endif // FLANG_RT_RUNTIME_INTERNAL_UNIT_H_
