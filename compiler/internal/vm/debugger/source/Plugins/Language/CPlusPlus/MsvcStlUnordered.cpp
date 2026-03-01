//===-- MsvcStlUnordered.cpp ----------------------------------------------===//
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
using namespace lldb_private;

namespace {

class UnorderedFrontEnd : public SyntheticChildrenFrontEnd {
public:
  UnorderedFrontEnd(ValueObject &valobj) : SyntheticChildrenFrontEnd(valobj) {
    Update();
  }

  llvm::Expected<size_t> GetIndexOfChildWithName(ConstString name) override {
    if (!m_list_sp)
      return llvm::createStringError("Missing _List");
    return m_list_sp->GetIndexOfChildWithName(name);
  }

  lldb::ChildCacheState Update() override;

  llvm::Expected<uint32_t> CalculateNumChildren() override {
    if (!m_list_sp)
      return llvm::createStringError("Missing _List");
    return m_list_sp->GetNumChildren();
  }

  ValueObjectSP GetChildAtIndex(uint32_t idx) override {
    if (!m_list_sp)
      return nullptr;
    return m_list_sp->GetChildAtIndex(idx);
  }

private:
  ValueObjectSP m_list_sp;
};

} // namespace

lldb::ChildCacheState UnorderedFrontEnd::Update() {
  m_list_sp = nullptr;
  ValueObjectSP list_sp = m_backend.GetChildMemberWithName("_List");
  if (!list_sp)
    return lldb::ChildCacheState::eRefetch;
  m_list_sp = list_sp->GetSyntheticValue();
  return lldb::ChildCacheState::eRefetch;
}

bool formatters::IsMsvcStlUnordered(ValueObject &valobj) {
  if (auto valobj_sp = valobj.GetNonSyntheticValue())
    return valobj_sp->GetChildMemberWithName("_List") != nullptr;
  return false;
}

SyntheticChildrenFrontEnd *formatters::MsvcStlUnorderedSyntheticFrontEndCreator(
    CXXSyntheticChildren *, lldb::ValueObjectSP valobj_sp) {
  if (valobj_sp)
    return new UnorderedFrontEnd(*valobj_sp);
  return nullptr;
}
