//===-- ThreadInfo.h -----------------------------------------------*- C++
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

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_THREADINFO_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_THREADINFO_H

namespace ThreadInfo {

class QoS {
public:
  QoS() : constant_name(), printable_name(), enum_value(UINT32_MAX) {}
  bool IsValid() { return enum_value != UINT32_MAX; }
  std::string constant_name;
  std::string printable_name;
  uint32_t enum_value;
};
}

#endif // LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_THREADINFO_H
