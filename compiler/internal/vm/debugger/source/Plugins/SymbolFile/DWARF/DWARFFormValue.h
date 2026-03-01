//===-- DWARFFormValue.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFFORMVALUE_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFFORMVALUE_H

#include "DWARFDataExtractor.h"
#include "llvm/DebugInfo/DWARF/DWARFFormValue.h"
#include <optional>

namespace lldb_private::plugin {
namespace dwarf {
class DWARFUnit;
class SymbolFileDWARF;
class DWARFDIE;

class DWARFFormValue {
public:
  typedef llvm::DWARFFormValue::ValueType ValueType;
  enum {
    eValueTypeInvalid = 0,
    eValueTypeUnsigned,
    eValueTypeSigned,
    eValueTypeCStr,
    eValueTypeBlock
  };

  DWARFFormValue() = default;
  DWARFFormValue(const DWARFUnit *unit) : m_unit(unit) {}
  DWARFFormValue(const DWARFUnit *unit, dw_form_t form)
      : m_unit(unit), m_form(form) {}
  const DWARFUnit *GetUnit() const { return m_unit; }
  void SetUnit(const DWARFUnit *unit) { m_unit = unit; }
  dw_form_t Form() const { return m_form; }
  dw_form_t &FormRef() { return m_form; }
  void SetForm(dw_form_t form) { m_form = form; }
  const ValueType &Value() const { return m_value; }
  ValueType &ValueRef() { return m_value; }
  void SetValue(const ValueType &val) { m_value = val; }

  void Dump(Stream &s) const;
  bool ExtractValue(const DWARFDataExtractor &data, lldb::offset_t *offset_ptr);
  const uint8_t *BlockData() const;
  static std::optional<uint8_t> GetFixedSize(dw_form_t form,
                                             const DWARFUnit *u);
  std::optional<uint8_t> GetFixedSize() const;
  DWARFDIE Reference() const;

  /// If this is a reference to another DIE, return the corresponding DWARFUnit
  /// and DIE offset such that Unit->GetDIE(offset) produces the desired DIE.
  /// Otherwise, a nullptr and unspecified offset are returned.
  std::pair<DWARFUnit *, uint64_t> ReferencedUnitAndOffset() const;

  uint64_t Reference(dw_offset_t offset) const;
  bool Boolean() const { return m_value.uval != 0; }
  uint64_t Unsigned() const { return m_value.uval; }
  void SetUnsigned(uint64_t uval) { m_value.uval = uval; }
  int64_t Signed() const { return m_value.sval; }
  void SetSigned(int64_t sval) { m_value.sval = sval; }
  const char *AsCString() const;
  dw_addr_t Address() const;
  bool IsValid() const { return m_form != 0; }
  bool SkipValue(const DWARFDataExtractor &debug_info_data,
                 lldb::offset_t *offset_ptr) const;
  static bool SkipValue(const dw_form_t form,
                        const DWARFDataExtractor &debug_info_data,
                        lldb::offset_t *offset_ptr, const DWARFUnit *unit);
  static bool IsBlockForm(const dw_form_t form);
  static bool IsDataForm(const dw_form_t form);
  static int Compare(const DWARFFormValue &a, const DWARFFormValue &b);
  void Clear();
  static bool FormIsSupported(dw_form_t form);

  // The following methods use LLVM naming convension in order to be are used by
  // LLVM libraries.
  std::optional<uint64_t> getAsUnsignedConstant() const;
  std::optional<int64_t> getAsSignedConstant() const;
  const char *getAsCString() const { return AsCString(); }

protected:
  // Compile unit where m_value was located.
  // It may be different from compile unit where m_value refers to.
  const DWARFUnit *m_unit = nullptr; // Unit for this form
  dw_form_t m_form = dw_form_t(0);   // Form for this value
  ValueType m_value;                 // Contains all data for the form
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFFORMVALUE_H
