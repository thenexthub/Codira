//===-- DumpRegisterInfo.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_DUMPREGISTERINFO_H
#define LLDB_CORE_DUMPREGISTERINFO_H

#include <stdint.h>
#include <utility>
#include <vector>

namespace lldb_private {

class Stream;
class RegisterContext;
struct RegisterInfo;
class RegisterFlags;

void DumpRegisterInfo(Stream &strm, RegisterContext &ctx,
                      const RegisterInfo &info, uint32_t terminal_width);

// For testing only. Use DumpRegisterInfo instead.
void DoDumpRegisterInfo(
    Stream &strm, const char *name, const char *alt_name, uint32_t byte_size,
    const std::vector<const char *> &invalidates,
    const std::vector<const char *> &read_from,
    const std::vector<std::pair<const char *, uint32_t>> &in_sets,
    const RegisterFlags *flags_type, uint32_t terminal_width);

} // namespace lldb_private

#endif // LLDB_CORE_DUMPREGISTERINFO_H
