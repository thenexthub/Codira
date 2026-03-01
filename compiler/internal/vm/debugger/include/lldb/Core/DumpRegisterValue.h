//===-- DumpRegisterValue.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_DUMPREGISTERVALUE_H
#define LLDB_CORE_DUMPREGISTERVALUE_H

#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-forward.h"
#include <cstdint>

namespace lldb_private {

class ExecutionContextScope;
class RegisterValue;
struct RegisterInfo;
class Stream;

// The default value of 0 for reg_name_right_align_at means no alignment at
// all.
// Set print_flags to true to print register fields if they are available.
// If you do so, target_sp must be non-null for it to work.
void DumpRegisterValue(const RegisterValue &reg_val, Stream &s,
                       const RegisterInfo &reg_info, bool prefix_with_name,
                       bool prefix_with_alt_name, lldb::Format format,
                       uint32_t reg_name_right_align_at = 0,
                       ExecutionContextScope *exe_scope = nullptr,
                       bool print_flags = false,
                       lldb::TargetSP target_sp = nullptr);

} // namespace lldb_private

#endif // LLDB_CORE_DUMPREGISTERVALUE_H
