//===-- StringConvert.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_STRINGCONVERT_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_STRINGCONVERT_H

#include <cstdint>

namespace StringConvert {

int64_t ToSInt64(const char *s, int64_t fail_value = 0, int base = 0,
                 bool *success_ptr = nullptr);

uint64_t ToUInt64(const char *s, uint64_t fail_value = 0, int base = 0,
                  bool *success_ptr = nullptr);

double ToDouble(const char *s, double fail_value = 0.0,
                bool *success_ptr = nullptr);

} // namespace StringConvert

#endif
