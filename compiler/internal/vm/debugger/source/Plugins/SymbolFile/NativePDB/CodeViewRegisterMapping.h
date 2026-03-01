//===-- CodeViewRegisterMapping.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_CODEVIEWREGISTERMAPPING_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_CODEVIEWREGISTERMAPPING_H

#include "llvm/DebugInfo/CodeView/CodeView.h"
#include "llvm/TargetParser/Triple.h"

namespace lldb_private {
namespace npdb {

uint32_t GetLLDBRegisterNumber(llvm::Triple::ArchType arch_type,
                               llvm::codeview::RegisterId register_id);
uint32_t GetRegisterSize(llvm::codeview::RegisterId register_id);

} // namespace npdb
} // namespace lldb_private

#endif
