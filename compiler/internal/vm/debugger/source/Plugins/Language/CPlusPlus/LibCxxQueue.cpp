//===-- LibCxxQueue.cpp ---------------------------------------------------===//
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

class QueueFrontEnd : public SyntheticChildrenFrontEnd {
public:
  QueueFrontEnd(ValueObject &valobj) : SyntheticChildrenFrontEnd(valobj) {
    Update();
  }

  llvm::Expected<size_t> GetIndexOfChildWithName(ConstString name) override {
    if (m_container_sp)
      return m_container_sp->GetIndexOfChildWithName(name);
    return llvm::createStringError("Type has no child named '%s'",
                                   name.AsCString());
  }

  lldb::ChildCacheState Update() override;

  llvm::Expected<uint32_t> CalculateNumChildren() override {
    return m_container_sp ? m_container_sp->GetNumChildren() : 0;
  }

  ValueObjectSP GetChildAtIndex(uint32_t idx) override {
    return m_container_sp ? m_container_sp->GetChildAtIndex(idx)
                          : nullptr;
  }

private:
  // The lifetime of a ValueObject and all its derivative ValueObjects
  // (children, clones, etc.) is managed by a ClusterManager. These
  // objects are only destroyed when every shared pointer to any of them
  // is destroyed, so we must not store a shared pointer to any ValueObject
  // derived from our backend ValueObject (since we're in the same cluster).
  ValueObject* m_container_sp = nullptr;
};
} // namespace

lldb::ChildCacheState QueueFrontEnd::Update() {
  m_container_sp = nullptr;
  ValueObjectSP c_sp = m_backend.GetChildMemberWithName("c");
  if (!c_sp)
    return lldb::ChildCacheState::eRefetch;
  m_container_sp = c_sp->GetSyntheticValue().get();
  return lldb::ChildCacheState::eRefetch;
}

SyntheticChildrenFrontEnd *
formatters::LibcxxQueueFrontEndCreator(CXXSyntheticChildren *,
                                       lldb::ValueObjectSP valobj_sp) {
  if (valobj_sp)
    return new QueueFrontEnd(*valobj_sp);
  return nullptr;
}
