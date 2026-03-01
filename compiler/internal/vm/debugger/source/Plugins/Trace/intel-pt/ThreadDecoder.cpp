//===-- ThreadDecoder.cpp --======-----------------------------------------===//
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

#include "ThreadDecoder.h"
#include "../common/ThreadPostMortemTrace.h"
#include "LibiptDecoder.h"
#include "TraceIntelPT.h"
#include "llvm/Support/MemoryBuffer.h"
#include <optional>
#include <utility>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::trace_intel_pt;
using namespace llvm;

ThreadDecoder::ThreadDecoder(const ThreadSP &thread_sp, TraceIntelPT &trace)
    : m_thread_sp(thread_sp), m_trace(trace) {}

Expected<std::optional<uint64_t>> ThreadDecoder::FindLowestTSC() {
  std::optional<uint64_t> lowest_tsc;
  Error err = m_trace.OnThreadBufferRead(
      m_thread_sp->GetID(), [&](llvm::ArrayRef<uint8_t> data) -> llvm::Error {
        Expected<std::optional<uint64_t>> tsc =
            FindLowestTSCInTrace(m_trace, data);
        if (!tsc)
          return tsc.takeError();
        lowest_tsc = *tsc;
        return Error::success();
      });
  if (err)
    return std::move(err);
  return lowest_tsc;
}

Expected<DecodedThreadSP> ThreadDecoder::Decode() {
  if (!m_decoded_thread.has_value()) {
    if (Expected<DecodedThreadSP> decoded_thread = DoDecode()) {
      m_decoded_thread = *decoded_thread;
    } else {
      return decoded_thread.takeError();
    }
  }
  return *m_decoded_thread;
}

llvm::Expected<DecodedThreadSP> ThreadDecoder::DoDecode() {
  return m_trace.GetThreadTimer(m_thread_sp->GetID())
      .TimeTask("Decoding instructions", [&]() -> Expected<DecodedThreadSP> {
        DecodedThreadSP decoded_thread_sp = std::make_shared<DecodedThread>(
            m_thread_sp, m_trace.GetPerfZeroTscConversion());

        Error err = m_trace.OnThreadBufferRead(
            m_thread_sp->GetID(), [&](llvm::ArrayRef<uint8_t> data) {
              return DecodeSingleTraceForThread(*decoded_thread_sp, m_trace,
                                                data);
            });

        if (err)
          return std::move(err);
        return decoded_thread_sp;
      });
}
