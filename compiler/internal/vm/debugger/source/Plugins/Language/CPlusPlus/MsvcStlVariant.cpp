//===-- MsvcStlVariant.cpp-------------------------------------------------===//
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

#include "MsvcStl.h"
#include "lldb/DataFormatters/FormattersHelpers.h"
#include "lldb/Symbol/CompilerType.h"
#include <optional>

using namespace lldb;
using namespace lldb_private;

namespace {

// A variant when using DWARF looks as follows:
// (lldb) fr v -R v1
// (std::variant<int, double, char>) v1 = {
//   std::_SMF_control<std::_Variant_base<int, double, char>, int, double, char>
//   = {
//     std::_Variant_storage<int, double, char> = {
//        = {
//         _Head = 0
//         _Tail = {
//            = {
//             _Head = 2
//             _Tail = {
//                = {
//                 _Head = '\0'
//                 _Tail = {}
//               }
//             }
//           }
//         }
//       }
//     }
//     _Which = '\x01'
//   }
// }

ValueObjectSP GetStorageMember(ValueObject &valobj, llvm::StringRef name) {
  // Find the union
  ValueObjectSP union_sp = valobj.GetChildAtIndex(0);
  if (!union_sp)
    return nullptr;
  return union_sp->GetChildMemberWithName(name);
}

ValueObjectSP GetHead(ValueObject &valobj) {
  return GetStorageMember(valobj, "_Head");
}
ValueObjectSP GetTail(ValueObject &valobj) {
  return GetStorageMember(valobj, "_Tail");
}

std::optional<int64_t> GetIndexValue(ValueObject &valobj) {
  ValueObjectSP index_sp = valobj.GetChildMemberWithName("_Which");
  if (!index_sp)
    return std::nullopt;

  return {index_sp->GetValueAsSigned(-1)};
}

ValueObjectSP GetNthStorage(ValueObject &outer, int64_t index) {
  // We need to find the std::_Variant_storage base class.

  // Navigate "down" to std::_Variant_base by finding the holder of "_Which".
  // This might be down a few levels if a variant member isn't trivially
  // destructible/copyable/etc.
  ValueObjectSP which_sp = outer.GetChildMemberWithName("_Which");
  if (!which_sp)
    return nullptr;
  ValueObject *parent = which_sp->GetParent();
  if (!parent)
    return nullptr;

  // Now go to std::_Variant_storage.
  ValueObjectSP container_sp = parent->GetChildAtIndex(0);
  if (!container_sp)
    return nullptr;

  for (int64_t i = 0; i < index; i++) {
    container_sp = GetTail(*container_sp);
    if (!container_sp)
      return nullptr;
  }
  return container_sp;
}

} // namespace

bool formatters::IsMsvcStlVariant(ValueObject &valobj) {
  if (auto valobj_sp = valobj.GetNonSyntheticValue()) {
    return valobj_sp->GetChildMemberWithName("_Which") != nullptr;
  }
  return false;
}

bool formatters::MsvcStlVariantSummaryProvider(
    ValueObject &valobj, Stream &stream, const TypeSummaryOptions &options) {
  ValueObjectSP valobj_sp(valobj.GetNonSyntheticValue());
  if (!valobj_sp)
    return false;

  auto index = GetIndexValue(*valobj_sp);
  if (!index)
    return false;

  if (*index < 0) {
    stream.Printf(" No Value");
    return true;
  }

  ValueObjectSP storage = GetNthStorage(*valobj_sp, *index);
  if (!storage)
    return false;
  CompilerType storage_type = storage->GetCompilerType();
  if (!storage_type)
    return false;
  // Resolve the typedef
  if (storage_type.IsTypedefType())
    storage_type = storage_type.GetTypedefedType();

  CompilerType active_type = storage_type.GetTypeTemplateArgument(1, true);
  if (!active_type) {
    // PDB: get the type from the head as we don't have template arguments
    // there.
    ValueObjectSP head = GetHead(*storage);
    if (!head)
      return false;
    active_type = head->GetCompilerType();
    if (!active_type)
      return false;
  }

  stream << " Active Type = " << active_type.GetDisplayTypeName() << " ";
  return true;
}

namespace {
class VariantFrontEnd : public SyntheticChildrenFrontEnd {
public:
  VariantFrontEnd(ValueObject &valobj) : SyntheticChildrenFrontEnd(valobj) {
    Update();
  }

  llvm::Expected<size_t> GetIndexOfChildWithName(ConstString name) override {
    auto optional_idx = formatters::ExtractIndexFromString(name.GetCString());
    if (!optional_idx) {
      return llvm::createStringError("Type has no child named '%s'",
                                     name.AsCString());
    }
    return *optional_idx;
  }

  lldb::ChildCacheState Update() override;
  llvm::Expected<uint32_t> CalculateNumChildren() override { return m_size; }
  ValueObjectSP GetChildAtIndex(uint32_t idx) override;

private:
  size_t m_size = 0;
};
} // namespace

lldb::ChildCacheState VariantFrontEnd::Update() {
  m_size = 0;

  auto index = GetIndexValue(m_backend);
  if (index && *index >= 0)
    m_size = 1;

  return lldb::ChildCacheState::eRefetch;
}

ValueObjectSP VariantFrontEnd::GetChildAtIndex(uint32_t idx) {
  if (idx >= m_size)
    return nullptr;

  auto index = GetIndexValue(m_backend);
  if (!index)
    return nullptr;

  ValueObjectSP storage_sp = GetNthStorage(m_backend, *index);
  if (!storage_sp)
    return nullptr;

  ValueObjectSP head_sp = GetHead(*storage_sp);
  if (!head_sp)
    return nullptr;

  return head_sp->Clone(ConstString("Value"));
}

SyntheticChildrenFrontEnd *formatters::MsvcStlVariantSyntheticFrontEndCreator(
    CXXSyntheticChildren *, lldb::ValueObjectSP valobj_sp) {
  if (valobj_sp)
    return new VariantFrontEnd(*valobj_sp);
  return nullptr;
}
