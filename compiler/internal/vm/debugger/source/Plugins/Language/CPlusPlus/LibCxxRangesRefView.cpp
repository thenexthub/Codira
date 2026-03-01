//===-- LibCxxRangesRefView.cpp -------------------------------------------===//
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
#include "lldb/Utility/ConstString.h"
#include "lldb/ValueObject/ValueObject.h"
#include "llvm/ADT/APSInt.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::formatters;

namespace lldb_private {
namespace formatters {

class LibcxxStdRangesRefViewSyntheticFrontEnd
    : public SyntheticChildrenFrontEnd {
public:
  LibcxxStdRangesRefViewSyntheticFrontEnd(lldb::ValueObjectSP valobj_sp);

  ~LibcxxStdRangesRefViewSyntheticFrontEnd() override = default;

  llvm::Expected<uint32_t> CalculateNumChildren() override {
    // __range_ will be the sole child of this type
    return 1;
  }

  lldb::ValueObjectSP GetChildAtIndex(uint32_t idx) override {
    // Since we only have a single child, return it
    assert(idx == 0);
    return m_range_sp;
  }

  lldb::ChildCacheState Update() override;

  llvm::Expected<size_t> GetIndexOfChildWithName(ConstString name) override {
    // We only have a single child
    return 0;
  }

private:
  /// Pointer to the dereferenced __range_ member
  lldb::ValueObjectSP m_range_sp = nullptr;
};

lldb_private::formatters::LibcxxStdRangesRefViewSyntheticFrontEnd::
    LibcxxStdRangesRefViewSyntheticFrontEnd(lldb::ValueObjectSP valobj_sp)
    : SyntheticChildrenFrontEnd(*valobj_sp) {
  if (valobj_sp)
    Update();
}

lldb::ChildCacheState
lldb_private::formatters::LibcxxStdRangesRefViewSyntheticFrontEnd::Update() {
  ValueObjectSP range_ptr =
      GetChildMemberWithName(m_backend, {ConstString("__range_")});
  if (!range_ptr)
    return lldb::ChildCacheState::eRefetch;

  lldb_private::Status error;
  m_range_sp = range_ptr->Dereference(error);

  return error.Success() ? lldb::ChildCacheState::eReuse
                         : lldb::ChildCacheState::eRefetch;
}

lldb_private::SyntheticChildrenFrontEnd *
LibcxxStdRangesRefViewSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                               lldb::ValueObjectSP valobj_sp) {
  if (!valobj_sp)
    return nullptr;
  CompilerType type = valobj_sp->GetCompilerType();
  if (!type.IsValid())
    return nullptr;
  return new LibcxxStdRangesRefViewSyntheticFrontEnd(valobj_sp);
}

} // namespace formatters
} // namespace lldb_private
