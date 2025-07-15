///===--- ExecutorChecks.cpp - Static assertions to check struct layouts ---===///
///
/// This source file is part of the Codira.org open source project
///
/// Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
/// Licensed under Apache License v2.0 with Runtime Library Exception
///
/// See https:///language.org/LICENSE.txt for license information
/// See https:///language.org/CONTRIBUTORS.txt for the list of Codira project authors
///
///===----------------------------------------------------------------------===///
///
/// This file is responsible for checking that the structures in ExecutorImpl.h
/// are laid out exactly the same as those in the ABI headers.
///
///===----------------------------------------------------------------------===///

#include "language/Runtime/Concurrency.h"

#include "language/ABI/Executor.h"
#include "language/ABI/MetadataValues.h"
#include "language/ABI/Task.h"

#include "ExecutorImpl.h"

// JobFlags
static_assert(sizeof(language::JobFlags) == sizeof(CodiraJobFlags));

// JobKind
static_assert(sizeof(language::JobKind) == sizeof(CodiraJobKind));
static_assert((CodiraJobKind)language::JobKind::Task == CodiraTaskJobKind);
static_assert((CodiraJobKind)language::JobKind::First_Reserved == CodiraFirstReservedJobKind);

// JobPriority
static_assert(sizeof(language::JobPriority) == sizeof(CodiraJobPriority));
static_assert((CodiraJobPriority)language::JobPriority::UserInteractive
              == CodiraUserInteractiveJobPriority);
static_assert((CodiraJobPriority)language::JobPriority::UserInteractive
              == CodiraUserInteractiveJobPriority);
static_assert((CodiraJobPriority)language::JobPriority::UserInitiated
              == CodiraUserInitiatedJobPriority);
static_assert((CodiraJobPriority)language::JobPriority::Default
              == CodiraDefaultJobPriority);
static_assert((CodiraJobPriority)language::JobPriority::Utility
              == CodiraUtilityJobPriority);
static_assert((CodiraJobPriority)language::JobPriority::Background
              == CodiraBackgroundJobPriority);
static_assert((CodiraJobPriority)language::JobPriority::Unspecified
              == CodiraUnspecifiedJobPriority);

// Job (has additional fields not exposed via CodiraJob)
static_assert(sizeof(language::Job) >= sizeof(CodiraJob));

// SerialExecutorRef
static_assert(sizeof(language::SerialExecutorRef) == sizeof(CodiraExecutorRef));

// language_clock_id
static_assert((CodiraClockId)language::language_clock_id_continuous
              == CodiraContinuousClock);
static_assert((CodiraClockId)language::language_clock_id_suspending ==
              CodiraSuspendingClock);
