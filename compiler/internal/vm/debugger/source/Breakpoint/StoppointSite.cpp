//===-- StoppointSite.cpp ---------------------------------------------===//
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

#include "lldb/Breakpoint/StoppointSite.h"


using namespace lldb;
using namespace lldb_private;

StoppointSite::StoppointSite(break_id_t id, addr_t addr, bool hardware)
    : m_id(id), m_addr(addr), m_is_hardware_required(hardware), m_byte_size(0),
      m_hit_counter() {}

StoppointSite::StoppointSite(break_id_t id, addr_t addr, uint32_t byte_size,
                             bool hardware)
    : m_id(id), m_addr(addr), m_is_hardware_required(hardware),
      m_byte_size(byte_size), m_hit_counter() {}
