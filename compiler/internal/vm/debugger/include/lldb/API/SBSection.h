//===-- SBSection.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSECTION_H
#define LLDB_API_SBSECTION_H

#include "lldb/API/SBData.h"
#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBSection {
public:
  SBSection();

  SBSection(const lldb::SBSection &rhs);

  ~SBSection();

  const lldb::SBSection &operator=(const lldb::SBSection &rhs);

  explicit operator bool() const;

  bool IsValid() const;

  const char *GetName();

  lldb::SBSection GetParent();

  lldb::SBSection FindSubSection(const char *sect_name);

  size_t GetNumSubSections();

  lldb::SBSection GetSubSectionAtIndex(size_t idx);

  lldb::addr_t GetFileAddress();

  lldb::addr_t GetLoadAddress(lldb::SBTarget &target);

  lldb::addr_t GetByteSize();

  uint64_t GetFileOffset();

  uint64_t GetFileByteSize();

  lldb::SBData GetSectionData();

  lldb::SBData GetSectionData(uint64_t offset, uint64_t size);

  SectionType GetSectionType();

  /// Gets the permissions (RWX) of the section of the object file
  ///
  /// Returns a mask of bits of enum lldb::Permissions for this section.
  /// Sections for which permissions are not defined, 0 is returned for
  /// them. The binary representation of this value corresponds to [XRW]
  /// i.e. for a section having read and execute permissions, the value
  /// returned is 6
  ///
  /// \return
  ///     Returns an unsigned value for Permissions for the section.
  uint32_t
  GetPermissions() const;

  /// Return the size of a target's byte represented by this section
  /// in numbers of host bytes. Note that certain architectures have
  /// varying minimum addressable unit (i.e. byte) size for their
  /// CODE or DATA buses.
  ///
  /// \return
  ///     The number of host (8-bit) bytes needed to hold a target byte
  uint32_t GetTargetByteSize();

  /// Return the alignment of the section in bytes
  ///
  /// \return
  ///     The alignment of the section in bytes
  uint32_t GetAlignment();

  bool operator==(const lldb::SBSection &rhs);

  bool operator!=(const lldb::SBSection &rhs);

  bool GetDescription(lldb::SBStream &description);

private:
  friend class SBAddress;
  friend class SBModule;
  friend class SBTarget;

  SBSection(const lldb::SectionSP &section_sp);

  lldb::SectionSP GetSP() const;

  void SetSP(const lldb::SectionSP &section_sp);

  lldb::SectionWP m_opaque_wp;
};

} // namespace lldb

#endif // LLDB_API_SBSECTION_H
