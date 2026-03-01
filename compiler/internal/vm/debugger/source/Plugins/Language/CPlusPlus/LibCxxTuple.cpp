//===-- LibCxxTuple.cpp ---------------------------------------------------===//
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

#include "LibCxx.h"
#include "lldb/DataFormatters/FormattersHelpers.h"

using namespace lldb;
using namespace lldb_private;

namespace {

class TupleFrontEnd: public SyntheticChildrenFrontEnd {
public:
  TupleFrontEnd(ValueObject &valobj) : SyntheticChildrenFrontEnd(valobj) {
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
  llvm::Expected<uint32_t> CalculateNumChildren() override {
    return m_elements.size();
  }
  ValueObjectSP GetChildAtIndex(uint32_t idx) override;

private:
  // The lifetime of a ValueObject and all its derivative ValueObjects
  // (children, clones, etc.) is managed by a ClusterManager. These
  // objects are only destroyed when every shared pointer to any of them
  // is destroyed, so we must not store a shared pointer to any ValueObject
  // derived from our backend ValueObject (since we're in the same cluster).
  std::vector<ValueObject*> m_elements;
  ValueObject* m_base = nullptr;
};
}

lldb::ChildCacheState TupleFrontEnd::Update() {
  m_elements.clear();
  m_base = nullptr;

  ValueObjectSP base_sp;
  base_sp = m_backend.GetChildMemberWithName("__base_");
  if (!base_sp) {
    // Pre r304382 name of the base element.
    base_sp = m_backend.GetChildMemberWithName("base_");
  }
  if (!base_sp)
    return lldb::ChildCacheState::eRefetch;
  m_base = base_sp.get();
  m_elements.assign(base_sp->GetCompilerType().GetNumDirectBaseClasses(),
                    nullptr);
  return lldb::ChildCacheState::eRefetch;
}

ValueObjectSP TupleFrontEnd::GetChildAtIndex(uint32_t idx) {
  if (idx >= m_elements.size())
    return ValueObjectSP();
  if (!m_base)
    return ValueObjectSP();
  if (m_elements[idx])
    return m_elements[idx]->GetSP();

  CompilerType holder_type =
      m_base->GetCompilerType().GetDirectBaseClassAtIndex(idx, nullptr);
  if (!holder_type)
    return ValueObjectSP();
  ValueObjectSP holder_sp = m_base->GetChildAtIndex(idx);
  if (!holder_sp)
    return ValueObjectSP();

  ValueObjectSP elem_sp = holder_sp->GetChildAtIndex(0);
  if (elem_sp)
    m_elements[idx] =
        elem_sp->Clone(ConstString(llvm::formatv("[{0}]", idx).str())).get();

  if (m_elements[idx])
    return m_elements[idx]->GetSP();
  return ValueObjectSP();
}

SyntheticChildrenFrontEnd *
formatters::LibcxxTupleFrontEndCreator(CXXSyntheticChildren *,
                                       lldb::ValueObjectSP valobj_sp) {
  if (valobj_sp)
    return new TupleFrontEnd(*valobj_sp);
  return nullptr;
}
