//===-- DWARFDefines.cpp --------------------------------------------------===//
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

#include "DWARFDefines.h"
#include "lldb/Utility/ConstString.h"
#include <cstdio>
#include <cstring>
#include <string>

namespace lldb_private::plugin {
namespace dwarf {

llvm::StringRef DW_TAG_value_to_name(dw_tag_t tag) {
  static constexpr llvm::StringLiteral s_unknown_tag_name("<unknown DW_TAG>");
  if (llvm::StringRef tag_name = llvm::dwarf::TagString(tag); !tag_name.empty())
    return tag_name;

  return s_unknown_tag_name;
}

} // namespace dwarf
} // namespace lldb_private::plugin
