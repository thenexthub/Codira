//===-- Instrumentation.cpp -----------------------------------------------===//
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

#include "lldb/Utility/Instrumentation.h"
#include "lldb/Utility/LLDBLog.h"
#include "llvm/Support/Signposts.h"

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <thread>

using namespace lldb_private;
using namespace lldb_private::instrumentation;

// Whether we're currently across the API boundary.
static thread_local bool g_global_boundary = false;

// Instrument SB API calls with signposts when supported.
static llvm::ManagedStatic<llvm::SignpostEmitter> g_api_signposts;

Instrumenter::Instrumenter(llvm::StringRef pretty_func,
                           std::string &&pretty_args)
    : m_pretty_func(pretty_func) {
  if (!g_global_boundary) {
    g_global_boundary = true;
    m_local_boundary = true;
    g_api_signposts->startInterval(this, m_pretty_func);
  }
  LLDB_LOG(GetLog(LLDBLog::API), "[{0}] {1} ({2})",
           m_local_boundary ? "external" : "internal", m_pretty_func,
           pretty_args);
}

Instrumenter::~Instrumenter() {
  if (m_local_boundary) {
    g_global_boundary = false;
    g_api_signposts->endInterval(this, m_pretty_func);
  }
}
