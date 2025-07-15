//===--- Debug.h - Codira Runtime debug helpers ------------------*- C++ -*-===//
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
// Random debug support
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_DEBUG_HELPERS_H
#define LANGUAGE_RUNTIME_DEBUG_HELPERS_H

#include "language/Runtime/Config.h"
#include "language/Basic/Unreachable.h"
#include <atomic>
#include <cstdarg>
#include <functional>
#include <stdint.h>

#ifdef LANGUAGE_HAVE_CRASHREPORTERCLIENT

#define CRASHREPORTER_ANNOTATIONS_VERSION 5
#define CRASHREPORTER_ANNOTATIONS_SECTION "__crash_info"

struct crashreporter_annotations_t {
  uint64_t version;          // unsigned long
  uint64_t message;          // char *
  uint64_t signature_string; // char *
  uint64_t backtrace;        // char *
  uint64_t message2;         // char *
  uint64_t thread;           // uint64_t
  uint64_t dialog_mode;      // unsigned int
  uint64_t abort_cause;      // unsigned int
};

extern "C" {
LANGUAGE_RUNTIME_LIBRARY_VISIBILITY
extern struct crashreporter_annotations_t gCRAnnotations;
}

LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline void CRSetCrashLogMessage(const char *message) {
  gCRAnnotations.message = reinterpret_cast<uint64_t>(message);
}

LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline const char *CRGetCrashLogMessage() {
  return reinterpret_cast<const char *>(gCRAnnotations.message);
}

#else

LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline void CRSetCrashLogMessage(const char *) {}

#endif

namespace language {

// Duplicated from Metadata.h. We want to use this header
// in places that cannot themselves include Metadata.h.
struct InProcess;
template <typename Runtime> struct TargetMetadata;
using Metadata = TargetMetadata<InProcess>;

// language::crash() halts with a crash log message, 
// but otherwise tries not to disturb register state.

LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE // Minimize trashed registers
static inline void crash(const char *message) {
  CRSetCrashLogMessage(message);

  LANGUAGE_RUNTIME_BUILTIN_TRAP;
  language_unreachable("Expected compiler to crash.");
}

// language::fatalErrorv() halts with a crash log message,
// but makes no attempt to preserve register state.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN
LANGUAGE_VFORMAT(2)
extern void fatalErrorv(uint32_t flags, const char *format, va_list args);

// language::fatalError() halts with a crash log message,
// but makes no attempt to preserve register state.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN
LANGUAGE_FORMAT(2, 3)
extern void
fatalError(uint32_t flags, const char *format, ...);

/// language::warning() emits a warning from the runtime.
extern void
LANGUAGE_VFORMAT(2)
warningv(uint32_t flags, const char *format, va_list args);

/// language::warning() emits a warning from the runtime.
extern void
LANGUAGE_FORMAT(2, 3)
warning(uint32_t flags, const char *format, ...);

// language_dynamicCastFailure halts using fatalError()
// with a description of a failed cast's types.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN
void
language_dynamicCastFailure(const Metadata *sourceType,
                         const Metadata *targetType,
                         const char *message = nullptr);

// language_dynamicCastFailure halts using fatalError()
// with a description of a failed cast's types.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN
void
language_dynamicCastFailure(const void *sourceType, const char *sourceName, 
                         const void *targetType, const char *targetName, 
                         const char *message = nullptr);

LANGUAGE_RUNTIME_EXPORT
void language_reportError(uint32_t flags, const char *message);

LANGUAGE_RUNTIME_EXPORT
void language_reportWarning(uint32_t flags, const char *message);

#if !defined(LANGUAGE_HAVE_CRASHREPORTERCLIENT)
LANGUAGE_RUNTIME_EXPORT
std::atomic<const char *> *language_getFatalErrorMessageBuffer();
#endif

// Halt due to an overflow in language_retain().
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
void language_abortRetainOverflow();

// Halt due to reading an unowned reference to a dead object.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
void language_abortRetainUnowned(const void *object);

// Halt due to an overflow in language_unownedRetain().
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
void language_abortUnownedRetainOverflow();

// Halt due to an overflow in incrementWeak().
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
void language_abortWeakRetainOverflow();

// Halt due to enabling an already enabled dynamic replacement().
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
void language_abortDynamicReplacementEnabling();

// Halt due to disabling an already disabled dynamic replacement().
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
void language_abortDynamicReplacementDisabling();

// Halt due to trying to use unicode data on platforms that don't have it.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
void language_abortDisabledUnicodeSupport();

// Halt due to a failure to allocate memory.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN
void language_abortAllocationFailure(size_t size, size_t alignMask);

/// This function dumps one line of a stack trace. It is assumed that \p framePC
/// is the address of the stack frame at index \p index. If \p shortOutput is
/// true, this functions prints only the name of the symbol and offset, ignores
/// \p index argument and omits the newline.
void dumpStackTraceEntry(unsigned index, void *framePC,
                         bool shortOutput = false);

LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
bool withCurrentBacktrace(std::function<void(void **, int)> call);

LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE
void printCurrentBacktrace(unsigned framesToSkip = 1);

/// Debugger breakpoint ABI. This structure is passed to the debugger (and needs
/// to be stable) and describes extra information about a fatal error or a
/// non-fatal warning, which should be logged as a runtime issue. Please keep
/// all integer values pointer-sized.
struct RuntimeErrorDetails {
  static const uintptr_t currentVersion = 2;

  // ABI version, needs to be set to "currentVersion".
  uintptr_t version;

  // A short hyphenated string describing the type of the issue, e.g.
  // "precondition-failed" or "exclusivity-violation".
  const char *errorType;

  // Description of the current thread's stack position.
  const char *currentStackDescription;

  // Number of frames in the current stack that should be ignored when reporting
  // the issue (excluding the reportToDebugger/_language_runtime_on_report frame).
  // The remaining top frame should point to user's code where the bug is.
  uintptr_t framesToSkip;

  // Address of some associated object (if there's any).
  const void *memoryAddress;

  // A structure describing an extra thread (and its stack) that is related.
  struct Thread {
    const char *description;
    uint64_t threadID;
    uintptr_t numFrames;
    void **frames;
  };

  // Number of extra threads (excluding the current thread) that are related,
  // and the pointer to the array of extra threads.
  uintptr_t numExtraThreads;
  Thread *threads;

  // Describes a suggested fix-it. Text in [startLine:startColumn,
  // endLine:endColumn) is to be replaced with replacementText.
  struct FixIt {
    const char *filename;
    uintptr_t startLine;
    uintptr_t startColumn;
    uintptr_t endLine;
    uintptr_t endColumn;
    const char *replacementText;
  };

  // Describes some extra information, possible with fix-its, about the current
  // runtime issue.
  struct Note {
    const char *description;
    uintptr_t numFixIts;
    FixIt *fixIts;
  };

  // Number of suggested fix-its, and the pointer to the array of them.
  uintptr_t numFixIts;
  FixIt *fixIts;

  // Number of related notes, and the pointer to the array of them.
  uintptr_t numNotes;
  Note *notes;
};

enum: uintptr_t {
  RuntimeErrorFlagNone = 0,
  RuntimeErrorFlagFatal = 1 << 0
};

/// Debugger hook. Calling this stops the debugger with a message and details
/// about the issues. Called by overlays.
LANGUAGE_RUNTIME_STDLIB_SPI
void _language_reportToDebugger(uintptr_t flags, const char *message,
                             RuntimeErrorDetails *details = nullptr);

LANGUAGE_RUNTIME_STDLIB_SPI
bool _language_reportFatalErrorsToDebugger;

LANGUAGE_RUNTIME_STDLIB_SPI
bool _language_shouldReportFatalErrorsToDebugger();

LANGUAGE_RUNTIME_STDLIB_SPI
bool _language_debug_metadataAllocationIterationEnabled;

LANGUAGE_RUNTIME_STDLIB_SPI
const void * const _language_debug_allocationPoolPointer;

LANGUAGE_RUNTIME_STDLIB_SPI
std::atomic<const void *> _language_debug_metadataAllocationBacktraceList;

LANGUAGE_RUNTIME_STDLIB_SPI
const void * const _language_debug_protocolConformanceStatePointer;

LANGUAGE_RUNTIME_STDLIB_SPI
const uint64_t _language_debug_multiPayloadEnumPointerSpareBitsMask;

// namespace language
}

#endif // LANGUAGE_RUNTIME_DEBUG_HELPERS_H
