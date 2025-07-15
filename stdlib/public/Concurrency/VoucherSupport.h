//===--- VoucherSupport.h - Support code for OS vouchers -----------*- C++ -*-//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Support code for interfacing with OS voucher calls.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CONCURRENCY_VOUCHERSUPPORT_H
#define LANGUAGE_CONCURRENCY_VOUCHERSUPPORT_H

#include "TaskPrivate.h"
#include "language/ABI/Task.h"
#include "language/Runtime/VoucherShims.h"
#include <optional>

namespace language {

/// A class which manages voucher adoption for Job and Task objects.
class VoucherManager {
  /// The original voucher that was set on the thread before Codira started
  /// doing async work. This must be restored on the thread after we finish
  /// async work.
  std::optional<voucher_t> OriginalVoucher;

public:
  VoucherManager() {
    LANGUAGE_TASK_DEBUG_LOG("[%p] Constructing VoucherManager", this);
  }

  /// Clean up after completing async work, restoring the original voucher on
  /// the current thread if necessary. This MUST be called before the
  /// VoucherManager object is destroyed. It may also be called in other
  /// places to restore the original voucher and reset the VoucherManager.
  void leave() {
    if (OriginalVoucher) {
      LANGUAGE_TASK_DEBUG_LOG("[%p] Restoring original voucher %p", this,
                           *OriginalVoucher);
      if (language_voucher_needs_adopt(*OriginalVoucher)) {
        auto previous = voucher_adopt(*OriginalVoucher);
        language_voucher_release(previous);
      } else {
        language_voucher_release(*OriginalVoucher);
      }
      OriginalVoucher = std::nullopt;
    } else
      LANGUAGE_TASK_DEBUG_LOG("[%p] Leaving empty VoucherManager", this);
  }

  ~VoucherManager() { assert(!OriginalVoucher); }

  /// Set up for a new Job by adopting its voucher on the current thread. This
  /// takes over ownership of the voucher from the Job object. For plain Jobs,
  /// this is permanent. For Tasks, the voucher must be restored using
  /// restoreVoucher if the task suspends.
  void swapToJob(Job *job) {
    LANGUAGE_TASK_DEBUG_LOG("[%p] Swapping jobs to %p", this, job);
    assert(job);
    assert(job->Voucher != LANGUAGE_DEAD_VOUCHER);

    voucher_t previous;
    if (language_voucher_needs_adopt(job->Voucher)) {
      // If we need to adopt the voucher, do so, and grab the old one.
      LANGUAGE_TASK_DEBUG_LOG("[%p] Swapping jobs to %p, adopting voucher %p",
                           this, job, job->Voucher);
      previous = voucher_adopt(job->Voucher);
    } else {
      // If we don't need to adopt the voucher, take the voucher out of Job
      // directly.
      LANGUAGE_TASK_DEBUG_LOG(
          "[%p] Swapping jobs to to %p, voucher %p does not need adoption",
          this, job, job->Voucher);
      previous = job->Voucher;
    }

    // Either way, we've taken ownership of the job's voucher, so mark the job
    // as having a dead voucher.
    job->Voucher = LANGUAGE_DEAD_VOUCHER;
    if (!OriginalVoucher) {
      // If we don't yet have an original voucher, then save the one we grabbed
      // above to restore later.
      OriginalVoucher = previous;
      LANGUAGE_TASK_DEBUG_LOG("[%p] Saved original voucher %p", this, previous);
    } else {
      // We already have an original voucher. The one we grabbed above is not
      // needed. We own it, so destroy it here.
      language_voucher_release(previous);
    }
  }

  // Take the current thread's adopted voucher and place it back into the task
  // that previously owned it, re-adopting the thread's original voucher.
  void restoreVoucher(AsyncTask *task) {
    LANGUAGE_TASK_DEBUG_LOG("[%p] Restoring %svoucher on task %p", this,
                         OriginalVoucher ? "" : "missing ", task);
    assert(OriginalVoucher);
    assert(task->Voucher == LANGUAGE_DEAD_VOUCHER);

    if (language_voucher_needs_adopt(*OriginalVoucher)) {
      // Adopt the execution thread's original voucher. The task's voucher is
      // the one currently adopted, and is returned by voucher_adopt.
      task->Voucher = voucher_adopt(*OriginalVoucher);
    } else {
      // No need to adopt the voucher, so we can take the one out of
      // OriginalVoucher and return it to the task.
      task->Voucher = *OriginalVoucher;
    }

    // We've given up ownership of OriginalVoucher, clear it out.
    OriginalVoucher = std::nullopt;
  }
};

} // end namespace language

#endif
