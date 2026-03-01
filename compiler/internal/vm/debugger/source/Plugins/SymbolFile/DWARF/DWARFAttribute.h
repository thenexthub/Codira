//===-- DWARFAttribute.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFATTRIBUTE_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFATTRIBUTE_H

#include "DWARFDefines.h"
#include "DWARFFormValue.h"
#include "llvm/ADT/SmallVector.h"
#include <vector>

namespace lldb_private::plugin {
namespace dwarf {
class DWARFUnit;

class DWARFAttribute {
public:
  DWARFAttribute(dw_attr_t attr, dw_form_t form,
                 DWARFFormValue::ValueType value)
      : m_attr(attr), m_form(form), m_value(value) {}

  dw_attr_t get_attr() const { return m_attr; }
  dw_form_t get_form() const { return m_form; }
  DWARFFormValue::ValueType get_value() const { return m_value; }
  void get(dw_attr_t &attr, dw_form_t &form,
           DWARFFormValue::ValueType &val) const {
    attr = m_attr;
    form = m_form;
    val = m_value;
  }

protected:
  dw_attr_t m_attr;
  dw_form_t m_form;
  DWARFFormValue::ValueType m_value;
};

class DWARFAttributes {
public:
  DWARFAttributes();
  ~DWARFAttributes();

  void Append(const DWARFFormValue &form_value, dw_offset_t attr_die_offset,
              dw_attr_t attr);
  DWARFUnit *CompileUnitAtIndex(uint32_t i) const { return m_infos[i].cu; }
  dw_offset_t DIEOffsetAtIndex(uint32_t i) const {
    return m_infos[i].die_offset;
  }
  dw_attr_t AttributeAtIndex(uint32_t i) const {
    return m_infos[i].attr.get_attr();
  }
  dw_form_t FormAtIndex(uint32_t i) const { return m_infos[i].attr.get_form(); }
  DWARFFormValue::ValueType ValueAtIndex(uint32_t i) const {
    return m_infos[i].attr.get_value();
  }
  bool ExtractFormValueAtIndex(uint32_t i, DWARFFormValue &form_value) const;
  DWARFDIE FormValueAsReferenceAtIndex(uint32_t i) const;
  DWARFDIE FormValueAsReference(dw_attr_t attr) const;
  uint32_t FindAttributeIndex(dw_attr_t attr) const;
  void Clear() { m_infos.clear(); }
  size_t Size() const { return m_infos.size(); }

protected:
  struct AttributeValue {
    DWARFUnit *cu; // Keep the compile unit with each attribute in
                   // case we have DW_FORM_ref_addr values
    dw_offset_t die_offset;
    DWARFAttribute attr;
  };
  typedef llvm::SmallVector<AttributeValue, 8> collection;
  collection m_infos;
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFATTRIBUTE_H
