//===--- ConcurrencyLayout.cpp - Concurrency Type Layout Checks -----------===//
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

#include "Concurrency/ExecutorImpl.h"
#include "runtime/GenericCacheEntry.h"
#include "language/ABI/TargetLayout.h"
#include "language/ABI/Task.h"
#include "gtest/gtest.h"

#include <memory>

namespace {
// Ensure that Job's layout is compatible with what Dispatch expects.
// Note: MinimalDispatchObjectHeader just has the fields we care about, it is
// not complete and should not be used for anything other than these asserts.
struct MinimalDispatchObjectHeader {
  const void *VTable;
  int Opaque0;
  int Opaque1;
  void *Linkage;
};
} // namespace

namespace language {
// NOTE: this is not possible to mark as `constexpr` as `reinterpret_cast` is
// not permitted in `constexpr` statements. Additionally, even if we were able
// to get away from the `reinterpret_cast`, we cannot de-reference the `nullptr`
// in a `constexpr` statement. Finally, because we are checking the offsets of
// members in non-standard layouts, we must delay this check until runtime as
// the static computation is not well defined by the language standard.
template <typename Type, typename PMFType>
size_t offset_of(PMFType Type::*member) {
  return reinterpret_cast<uintptr_t>(
      std::addressof(static_cast<Type *>(nullptr)->*member));
}

#define offset_of(T, d) language::offset_of(&T::d)
} // namespace language

TEST(ConcurrencyLayout, JobLayout) {
  ASSERT_EQ(offset_of(language::Job, metadata), offset_of(CodiraJob, metadata));
#if !LANGUAGE_CONCURRENCY_EMBEDDED
  ASSERT_EQ(offset_of(language::Job, refCounts), offset_of(CodiraJob, refCounts));
#else
  ASSERT_EQ(offset_of(language::Job, embeddedRefcount),
            offset_of(CodiraJob, refCounts));
#endif
  ASSERT_EQ(offset_of(language::Job, SchedulerPrivate),
            offset_of(CodiraJob, schedulerPrivate));
  ASSERT_EQ(offset_of(language::Job, SchedulerPrivate) +
                (sizeof(*language::Job::SchedulerPrivate) * 0),
            offset_of(CodiraJob, schedulerPrivate) +
                (sizeof(*CodiraJob::schedulerPrivate) * 0));
  ASSERT_EQ(offset_of(language::Job, SchedulerPrivate) +
                (sizeof(*language::Job::SchedulerPrivate) * 1),
            offset_of(CodiraJob, schedulerPrivate) +
                (sizeof(*CodiraJob::schedulerPrivate) * 1));
  ASSERT_EQ(offset_of(language::Job, Flags), offset_of(CodiraJob, flags));

  // Job Metadata field must match location of Dispatch VTable field.
  ASSERT_EQ(offset_of(language::Job, metadata),
            offset_of(MinimalDispatchObjectHeader, VTable));

  // Dispatch Linkage field must match Job
  // SchedulerPrivate[DispatchLinkageIndex].
  ASSERT_EQ(offset_of(language::Job, SchedulerPrivate) +
                (sizeof(*language::Job::SchedulerPrivate) *
                 language::Job::DispatchLinkageIndex),
            offset_of(MinimalDispatchObjectHeader, Linkage));
}

TEST(MetadataLayout, CacheEntryLayout) {
// error: 'Value' is a private member of
// 'language::VariadicMetadataCacheEntryBase<language::GenericCacheEntry>'
#if 0
  ASSERT_EQ(
      offset_of(language::GenericCacheEntry, Value),
      offset_of(language::GenericMetadataCacheEntry<language::InProcess::StoredPointer>,
                Value));
#endif
}
