//===-- VectorIterator.h ----------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_DATAFORMATTERS_VECTORITERATOR_H
#define LLDB_DATAFORMATTERS_VECTORITERATOR_H

#include "lldb/lldb-forward.h"

#include "lldb/DataFormatters/TypeSynthetic.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Utility/ConstString.h"
#include "llvm/ADT/SmallVector.h"

namespace lldb_private {
namespace formatters {
class VectorIteratorSyntheticFrontEnd : public SyntheticChildrenFrontEnd {
public:
  VectorIteratorSyntheticFrontEnd(lldb::ValueObjectSP valobj_sp,
                                  llvm::ArrayRef<ConstString> item_names);

  llvm::Expected<uint32_t> CalculateNumChildren() override;

  lldb::ValueObjectSP GetChildAtIndex(uint32_t idx) override;

  lldb::ChildCacheState Update() override;

  llvm::Expected<size_t> GetIndexOfChildWithName(ConstString name) override;

private:
  ExecutionContextRef m_exe_ctx_ref;
  llvm::SmallVector<ConstString, 2> m_item_names;
  lldb::ValueObjectSP m_item_sp;
};

} // namespace formatters
} // namespace lldb_private

#endif // LLDB_DATAFORMATTERS_VECTORITERATOR_H
