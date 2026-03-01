//===-- DWARFLocationExpression.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_DWARFLOCATIONEXPRESSION_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_DWARFLOCATIONEXPRESSION_H

#include "lldb/lldb-forward.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/DebugInfo/CodeView/CodeView.h"
#include <map>

namespace llvm {
class APSInt;
class StringRef;
namespace codeview {
class TypeIndex;
}
namespace pdb {
class TpiStream;
}
} // namespace llvm
namespace lldb_private {
namespace npdb {
struct MemberValLocation {
  uint16_t reg_id;
  uint16_t reg_offset;
  bool is_at_reg = true;
};

DWARFExpression
MakeEnregisteredLocationExpression(llvm::codeview::RegisterId reg,
                                   lldb::ModuleSP module);

DWARFExpression MakeRegRelLocationExpression(llvm::codeview::RegisterId reg,
                                             int32_t offset,
                                             lldb::ModuleSP module);
DWARFExpression MakeVFrameRelLocationExpression(llvm::StringRef fpo_program,
                                                int32_t offset,
                                                lldb::ModuleSP module);
DWARFExpression MakeGlobalLocationExpression(uint16_t section, uint32_t offset,
                                             lldb::ModuleSP module);
DWARFExpression MakeConstantLocationExpression(
    llvm::codeview::TypeIndex underlying_ti, llvm::pdb::TpiStream &tpi,
    const llvm::APSInt &constant, lldb::ModuleSP module);
DWARFExpression MakeEnregisteredLocationExpressionForComposite(
    const std::map<uint64_t, MemberValLocation> &offset_to_location,
    std::map<uint64_t, size_t> &offset_to_size, size_t total_size,
    lldb::ModuleSP module);
} // namespace npdb
} // namespace lldb_private

#endif
