//===-- SBAddressRange.cpp ------------------------------------------------===//
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

#include "lldb/API/SBAddressRange.h"
#include "Utils.h"
#include "lldb/API/SBAddress.h"
#include "lldb/API/SBStream.h"
#include "lldb/API/SBTarget.h"
#include "lldb/Core/AddressRange.h"
#include "lldb/Core/Section.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/Utility/Stream.h"
#include <cstddef>
#include <memory>

using namespace lldb;
using namespace lldb_private;

SBAddressRange::SBAddressRange()
    : m_opaque_up(std::make_unique<AddressRange>()) {
  LLDB_INSTRUMENT_VA(this);
}

SBAddressRange::SBAddressRange(const SBAddressRange &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  m_opaque_up = clone(rhs.m_opaque_up);
}

SBAddressRange::SBAddressRange(lldb::SBAddress addr, lldb::addr_t byte_size)
    : m_opaque_up(std::make_unique<AddressRange>(addr.ref(), byte_size)) {
  LLDB_INSTRUMENT_VA(this, addr, byte_size);
}

SBAddressRange::~SBAddressRange() = default;

const SBAddressRange &SBAddressRange::operator=(const SBAddressRange &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_up = clone(rhs.m_opaque_up);
  return *this;
}

bool SBAddressRange::operator==(const SBAddressRange &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  return ref().operator==(rhs.ref());
}

bool SBAddressRange::operator!=(const SBAddressRange &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  return !(*this == rhs);
}

void SBAddressRange::Clear() {
  LLDB_INSTRUMENT_VA(this);

  ref().Clear();
}

bool SBAddressRange::IsValid() const {
  LLDB_INSTRUMENT_VA(this);

  return ref().IsValid();
}

lldb::SBAddress SBAddressRange::GetBaseAddress() const {
  LLDB_INSTRUMENT_VA(this);

  return lldb::SBAddress(ref().GetBaseAddress());
}

lldb::addr_t SBAddressRange::GetByteSize() const {
  LLDB_INSTRUMENT_VA(this);

  return ref().GetByteSize();
}

bool SBAddressRange::GetDescription(SBStream &description,
                                    const SBTarget target) {
  LLDB_INSTRUMENT_VA(this, description, target);

  return ref().GetDescription(&description.ref(), target.GetSP().get());
}

lldb_private::AddressRange &SBAddressRange::ref() const {
  assert(m_opaque_up && "opaque pointer must always be valid");
  return *m_opaque_up;
}
