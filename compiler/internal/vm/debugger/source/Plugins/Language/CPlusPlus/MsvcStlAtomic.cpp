//===-- MsvcStlAtomic.cpp -------------------------------------------------===//
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

#include "lldb/DataFormatters/TypeSynthetic.h"

using namespace lldb;

namespace lldb_private {
namespace formatters {

class MsvcStlAtomicSyntheticFrontEnd : public SyntheticChildrenFrontEnd {
public:
  MsvcStlAtomicSyntheticFrontEnd(lldb::ValueObjectSP valobj_sp);

  llvm::Expected<uint32_t> CalculateNumChildren() override;

  lldb::ValueObjectSP GetChildAtIndex(uint32_t idx) override;

  lldb::ChildCacheState Update() override;

  llvm::Expected<size_t> GetIndexOfChildWithName(ConstString name) override;

private:
  ValueObject *m_storage = nullptr;
  CompilerType m_element_type;
};

} // namespace formatters
} // namespace lldb_private

lldb_private::formatters::MsvcStlAtomicSyntheticFrontEnd::
    MsvcStlAtomicSyntheticFrontEnd(lldb::ValueObjectSP valobj_sp)
    : SyntheticChildrenFrontEnd(*valobj_sp), m_element_type() {
  if (valobj_sp)
    Update();
}

llvm::Expected<uint32_t> lldb_private::formatters::
    MsvcStlAtomicSyntheticFrontEnd::CalculateNumChildren() {
  return m_storage ? 1 : 0;
}

lldb::ValueObjectSP
lldb_private::formatters::MsvcStlAtomicSyntheticFrontEnd::GetChildAtIndex(
    uint32_t idx) {
  if (idx == 0 && m_storage && m_element_type.IsValid())
    return m_storage->Cast(m_element_type)->Clone(ConstString("Value"));
  return nullptr;
}

lldb::ChildCacheState
lldb_private::formatters::MsvcStlAtomicSyntheticFrontEnd::Update() {
  m_storage = nullptr;
  m_element_type.Clear();

  ValueObjectSP storage_sp = m_backend.GetChildMemberWithName("_Storage");
  if (!storage_sp)
    return lldb::ChildCacheState::eRefetch;

  CompilerType backend_type = m_backend.GetCompilerType();
  if (!backend_type)
    return lldb::ChildCacheState::eRefetch;

  m_element_type = backend_type.GetTypeTemplateArgument(0);
  if (!m_element_type) {
    // PDB doesn't have info about templates, so use value_type which equals T.
    m_element_type = backend_type.GetDirectNestedTypeWithName("value_type");

    if (!m_element_type)
      return lldb::ChildCacheState::eRefetch;
  }

  m_storage = storage_sp.get();
  return lldb::ChildCacheState::eRefetch;
}

llvm::Expected<size_t> lldb_private::formatters::
    MsvcStlAtomicSyntheticFrontEnd::GetIndexOfChildWithName(ConstString name) {
  if (name == "Value")
    return 0;
  return llvm::createStringError("Type has no child named '%s'",
                                 name.AsCString());
}

lldb_private::SyntheticChildrenFrontEnd *
lldb_private::formatters::MsvcStlAtomicSyntheticFrontEndCreator(
    CXXSyntheticChildren *, lldb::ValueObjectSP valobj_sp) {
  if (valobj_sp && IsMsvcStlAtomic(*valobj_sp))
    return new MsvcStlAtomicSyntheticFrontEnd(valobj_sp);
  return nullptr;
}

bool lldb_private::formatters::MsvcStlAtomicSummaryProvider(
    ValueObject &valobj, Stream &stream, const TypeSummaryOptions &options) {
  ValueObjectSP synth_sp = valobj.GetSyntheticValue();
  if (!synth_sp)
    return false;

  ValueObjectSP value_sp = synth_sp->GetChildAtIndex(0);
  std::string summary;
  if (value_sp->GetSummaryAsCString(summary, options) && !summary.empty()) {
    stream << summary;
    return true;
  }
  return false;
}

bool lldb_private::formatters::IsMsvcStlAtomic(ValueObject &valobj) {
  if (auto valobj_sp = valobj.GetNonSyntheticValue())
    return valobj_sp->GetChildMemberWithName("_Storage") != nullptr;
  return false;
}
