//===-- Progress.cpp ------------------------------------------------------===//
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

#include "lldb/Core/Progress.h"

#include "lldb/Core/Debugger.h"
#include "lldb/Utility/StreamString.h"
#include "llvm/Support/Signposts.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>

using namespace lldb;
using namespace lldb_private;

std::atomic<uint64_t> Progress::g_id(0);

// Instrument progress events with signposts when supported.
static llvm::ManagedStatic<llvm::SignpostEmitter> g_progress_signposts;

Progress::Progress(std::string title, std::string details,
                   std::optional<uint64_t> total,
                   lldb_private::Debugger *debugger,
                   Timeout<std::nano> minimum_report_time,
                   Progress::Origin origin)
    : m_total(total.value_or(Progress::kNonDeterministicTotal)),
      m_minimum_report_time(minimum_report_time), m_title(title),
      m_progress_id(++g_id),
      m_debugger_id(debugger ? std::optional<user_id_t>(debugger->GetID())
                             : std::nullopt),
      m_origin(origin),
      m_last_report_time_ns(
          std::chrono::nanoseconds(
              std::chrono::steady_clock::now().time_since_epoch())
              .count()),
      m_details(std::move(details)) {
  std::lock_guard<std::mutex> guard(m_mutex);
  ReportProgress();

  // Start signpost interval right before the meaningful work starts.
  g_progress_signposts->startInterval(this, m_title);
}

Progress::~Progress() {
  // End signpost interval as soon as possible.
  g_progress_signposts->endInterval(this, m_title);

  // Make sure to always report progress completed when this object is
  // destructed so it indicates the progress dialog/activity should go away.
  std::lock_guard<std::mutex> guard(m_mutex);
  m_completed = m_total;
  ReportProgress();
}

void Progress::Increment(uint64_t amount,
                         std::optional<std::string> updated_detail) {
  if (amount == 0)
    return;

  m_completed.fetch_add(amount, std::memory_order_relaxed);

  if (m_minimum_report_time) {
    using namespace std::chrono;

    nanoseconds now;
    uint64_t last_report_time_ns =
        m_last_report_time_ns.load(std::memory_order_relaxed);

    do {
      now = steady_clock::now().time_since_epoch();
      if (now < nanoseconds(last_report_time_ns) + *m_minimum_report_time)
        return; // Too little time has passed since the last report.

    } while (!m_last_report_time_ns.compare_exchange_weak(
        last_report_time_ns, now.count(), std::memory_order_relaxed,
        std::memory_order_relaxed));
  }

  std::lock_guard<std::mutex> guard(m_mutex);
  if (updated_detail)
    m_details = std::move(updated_detail.value());
  ReportProgress();
}

void Progress::ReportProgress() {
  // NB: Comparisons with optional<T> rely on the fact that std::nullopt is
  // "smaller" than zero.
  if (m_prev_completed >= m_total)
    return; // We've reported completion already.

  uint64_t completed =
      std::min(m_completed.load(std::memory_order_relaxed), m_total);
  if (completed < m_prev_completed)
    return; // An overflow in the m_completed counter. Just ignore these events.

  // Change the category bit if we're an internal or external progress.
  uint32_t progress_category_bit = m_origin == Progress::Origin::eExternal
                                       ? lldb::eBroadcastBitExternalProgress
                                       : lldb::eBroadcastBitProgress;

  Debugger::ReportProgress(m_progress_id, m_title, m_details, completed,
                           m_total, m_debugger_id, progress_category_bit);
  m_prev_completed = completed;
}
