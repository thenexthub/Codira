//===--- Errors.cpp - Error reporting utilities ---------------------------===//
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
// Utilities for reporting errors to stderr, system console, and crash logs.
//
//===----------------------------------------------------------------------===//

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#pragma comment(lib, "User32.Lib")

#include <mutex>
#endif

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <io.h>
#endif
#include <stdarg.h>

#include "ImageInspection.h"
#include "language/Demangling/Demangle.h"
#include "language/Runtime/Atomic.h"
#include "language/Runtime/Debug.h"
#include "language/Runtime/Portability.h"
#include "language/Runtime/Win32.h"
#include "language/Threading/Errors.h"
#include "language/Threading/Mutex.h"
#include "toolchain/ADT/StringRef.h"

#if defined(_MSC_VER)
#include <DbgHelp.h>
#else
#include <cxxabi.h>
#endif

#if __has_include(<execinfo.h>)
#include <execinfo.h>
#endif

#if LANGUAGE_STDLIB_HAS_ASL
#include <asl.h>
#elif defined(__ANDROID__)
#include <android/log.h>
#endif

#if defined(__ELF__)
#include <unwind.h>
#endif

#include <inttypes.h>

#ifdef LANGUAGE_HAVE_CRASHREPORTERCLIENT
#include <malloc/malloc.h>
#else
static std::atomic<const char *> kFatalErrorMessage;
#endif // LANGUAGE_HAVE_CRASHREPORTERCLIENT

#include "BacktracePrivate.h"

#include <atomic>

namespace FatalErrorFlags {
enum: uint32_t {
  ReportBacktrace = 1 << 0
};
} // end namespace FatalErrorFlags

using namespace language;

#if LANGUAGE_STDLIB_SUPPORTS_BACKTRACE_REPORTING && LANGUAGE_STDLIB_HAS_DLADDR
static bool getSymbolNameAddr(toolchain::StringRef libraryName,
                              const SymbolInfo &syminfo,
                              std::string &symbolName, uintptr_t &addrOut) {
  // If we failed to find a symbol and thus dlinfo->dli_sname is nullptr, we
  // need to use the hex address.
  bool hasUnavailableAddress = syminfo.getSymbolName() == nullptr;

  if (hasUnavailableAddress) {
    return false;
  }

  // Ok, now we know that we have some sort of "real" name. Set the outAddr.
  addrOut = uintptr_t(syminfo.getSymbolAddress());

  // First lets try to demangle using cxxabi. If this fails, we will try to
  // demangle with language. We are taking advantage of __cxa_demangle actually
  // providing failure status instead of just returning the original string like
  // language demangle.
#if defined(_WIN32)
  const char *szSymbolName = syminfo.getSymbolName();

  // UnDecorateSymbolName() will not fail for Codira symbols, so detect them
  // up-front and let Codira handle them.
  if (!Demangle::isMangledName(szSymbolName)) {
    char szUndName[1024];
    DWORD dwResult;
    dwResult = _language_win32_withDbgHelpLibrary([&] (HANDLE hProcess) -> DWORD {
      if (!hProcess) {
        return 0;
      }

      DWORD dwFlags = UNDNAME_COMPLETE;
#if !defined(_WIN64)
      dwFlags |= UNDNAME_32_BIT_DECODE;
#endif

      return UnDecorateSymbolName(szSymbolName, szUndName,
                                  sizeof(szUndName), dwFlags);
    });

    if (dwResult) {
      symbolName += szUndName;
      return true;
    }
  }
#else
  int status;
  char *demangled =
      abi::__cxa_demangle(syminfo.getSymbolName(), 0, 0, &status);
  if (status == 0) {
    assert(demangled != nullptr &&
           "If __cxa_demangle succeeds, demangled should never be nullptr");
    symbolName += demangled;
    free(demangled);
    return true;
  }
  assert(demangled == nullptr &&
         "If __cxa_demangle fails, demangled should be a nullptr");
#endif

  // Otherwise, try to demangle with language. If language fails to demangle, it will
  // just pass through the original output.
  symbolName = demangleSymbolAsString(
      syminfo.getSymbolName(), strlen(syminfo.getSymbolName()),
      Demangle::DemangleOptions::SimplifiedUIDemangleOptions());
  return true;
}
#endif

void language::dumpStackTraceEntry(unsigned index, void *framePC,
                                bool shortOutput) {
#if LANGUAGE_STDLIB_SUPPORTS_BACKTRACE_REPORTING && LANGUAGE_STDLIB_HAS_DLADDR
  auto syminfo = SymbolInfo::lookup(framePC);
  if (!syminfo.has_value()) {
    constexpr const char *format = "%-4u %-34s 0x%0.16tx\n";
    fprintf(stderr, format, index, "<unknown>",
            reinterpret_cast<uintptr_t>(framePC));
    return;
  }

  // If SymbolInfo:lookup succeeded then fileName is non-null. Thus, we find the
  // library name here. Avoid using StringRef::rsplit because its definition
  // is not provided in the header so that it requires linking with
  // libSupport.a.
  toolchain::StringRef libraryName{syminfo->getFilename()};

#ifdef _WIN32
  libraryName = libraryName.substr(libraryName.rfind('\\')).substr(1);
#else
  libraryName = libraryName.substr(libraryName.rfind('/')).substr(1);
#endif

  // Next we get the symbol name that we are going to use in our backtrace.
  std::string symbolName;
  // We initialize symbolAddr to framePC so that if we succeed in finding the
  // symbol, we get the offset in the function and if we fail to find the symbol
  // we just get HexAddr + 0.
  uintptr_t symbolAddr = uintptr_t(framePC);
  bool foundSymbol =
      getSymbolNameAddr(libraryName, syminfo.value(), symbolName, symbolAddr);
  ptrdiff_t offset = 0;
  if (foundSymbol) {
    offset = ptrdiff_t(uintptr_t(framePC) - symbolAddr);
  } else {
    auto baseAddress = syminfo->getBaseAddress();
    offset = ptrdiff_t(uintptr_t(framePC) - uintptr_t(baseAddress));
    symbolAddr = uintptr_t(framePC);
    symbolName = "<unavailable>";
  }

  const char *libraryNameStr = libraryName.data();

  if (!libraryNameStr)
    libraryNameStr = "<unknown>";

  // We do not use %p here for our pointers since the format is implementation
  // defined. This makes it logically impossible to check the output. Forcing
  // hexadecimal solves this issue.
  // If the symbol is not available, we print out <unavailable> + offset
  // from the base address of where the image containing framePC is mapped.
  // This gives enough info to reconstruct identical debugging target after
  // this process terminates.
  if (shortOutput) {
    fprintf(stderr, "%s`%s + %td", libraryNameStr, symbolName.c_str(),
            offset);
  } else {
    constexpr const char *format = "%-4u %-34s 0x%0.16" PRIxPTR " %s + %td\n";
    fprintf(stderr, format, index, libraryNameStr, symbolAddr,
            symbolName.c_str(), offset);
  }
#else
  if (shortOutput) {
    fprintf(stderr, "<unavailable>");
  } else {
    constexpr const char *format = "%-4u 0x%0.16tx\n";
    fprintf(stderr, format, index, reinterpret_cast<uintptr_t>(framePC));
  }
#endif
}

#if defined(__ELF__)
struct UnwindState {
  void **current;
  void **end;
};

static _Unwind_Reason_Code CodiraUnwindFrame(struct _Unwind_Context *context, void *arg) {
  struct UnwindState *state = static_cast<struct UnwindState *>(arg);
  if (state->current == state->end) {
    return _URC_END_OF_STACK;
  }

  uintptr_t pc;
#if defined(__arm__)
  // ARM r15 is PC.  UNW_REG_PC is *not* the same value, and using that will
  // result in abnormal behaviour.
  _Unwind_VRS_Get(context, _UVRSC_CORE, 15, _UVRSD_UINT32, &pc);
  // Clear the ISA bit during the reporting.
  pc &= ~(uintptr_t)0x1;
#else
  pc = _Unwind_GetIP(context);
#endif
  if (pc) {
    *state->current++ = reinterpret_cast<void *>(pc);
  }
  return _URC_NO_REASON;
}
#endif

LANGUAGE_ALWAYS_INLINE
static bool withCurrentBacktraceImpl(std::function<void(void **, int)> call) {
#if LANGUAGE_STDLIB_SUPPORTS_BACKTRACE_REPORTING
  constexpr unsigned maxSupportedStackDepth = 128;
  void *addrs[maxSupportedStackDepth];
#if defined(_WIN32)
  int symbolCount = CaptureStackBackTrace(0, maxSupportedStackDepth, addrs, NULL);
#elif defined(__ELF__)
  struct UnwindState state = {&addrs[0], &addrs[maxSupportedStackDepth]};
  _Unwind_Backtrace(CodiraUnwindFrame, &state);
  int symbolCount = state.current - addrs;
#else
  int symbolCount = backtrace(addrs, maxSupportedStackDepth);
#endif
  call(addrs, symbolCount);
  return true;
#else
  return false;
#endif
}

LANGUAGE_NOINLINE
bool language::withCurrentBacktrace(std::function<void(void **, int)> call) {
  return withCurrentBacktraceImpl(call);
}

LANGUAGE_NOINLINE
void language::printCurrentBacktrace(unsigned framesToSkip) {
  bool success = withCurrentBacktraceImpl([&](void **addrs, int symbolCount) {
    for (int i = framesToSkip; i < symbolCount; ++i) {
      dumpStackTraceEntry(i - framesToSkip, addrs[i]);
    }
  });
  if (!success)
    fprintf(stderr, "<backtrace unavailable>\n");
}

// Report a message to any forthcoming crash log.
static void
reportOnCrash(uint32_t flags, const char *message)
{
#ifdef LANGUAGE_HAVE_CRASHREPORTERCLIENT
  char *oldMessage = nullptr;
  char *newMessage = nullptr;

  oldMessage = std::atomic_load_explicit(
    (volatile std::atomic<char *> *)&gCRAnnotations.message,
    LANGUAGE_MEMORY_ORDER_CONSUME);

  do {
    if (newMessage) {
      free(newMessage);
      newMessage = nullptr;
    }

    if (oldMessage) {
      language_asprintf(&newMessage, "%s%s", oldMessage, message);
    } else {
      newMessage = strdup(message);
    }
  } while (!std::atomic_compare_exchange_strong_explicit(
             (volatile std::atomic<char *> *)&gCRAnnotations.message,
             &oldMessage, newMessage,
             std::memory_order_release,
             LANGUAGE_MEMORY_ORDER_CONSUME));
#else
  const char *previous = nullptr;
  char *current = nullptr;
  previous =
      std::atomic_load_explicit(&kFatalErrorMessage, LANGUAGE_MEMORY_ORDER_CONSUME);

  do {
    ::free(current);
    current = nullptr;

    if (previous)
      language_asprintf(&current, "%s%s", current, message);
    else
#if defined(_WIN32)
      current = ::_strdup(message);
#else
      current = ::strdup(message);
#endif
  } while (!std::atomic_compare_exchange_strong_explicit(&kFatalErrorMessage,
                                                         &previous,
                                                         static_cast<const char *>(current),
                                                         std::memory_order_release,
                                                         LANGUAGE_MEMORY_ORDER_CONSUME));
#endif // LANGUAGE_HAVE_CRASHREPORTERCLIENT
}

// Report a message to system console and stderr.
static void
reportNow(uint32_t flags, const char *message)
{
#if defined(_WIN32)
#define STDERR_FILENO 2
  _write(STDERR_FILENO, message, strlen(message));
#else
  fputs(message, stderr);
  fflush(stderr);
#endif
#if LANGUAGE_STDLIB_HAS_ASL
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  asl_log(nullptr, nullptr, ASL_LEVEL_ERR, "%s", message);
#pragma clang diagnostic pop
#elif defined(__ANDROID__)
  __android_log_print(ANDROID_LOG_FATAL, "CodiraRuntime", "%s", message);
#endif
#if LANGUAGE_STDLIB_SUPPORTS_BACKTRACE_REPORTING
  if (flags & FatalErrorFlags::ReportBacktrace) {
    fputs("Current stack trace:\n", stderr);
    printCurrentBacktrace();
  }
#endif
}

LANGUAGE_NOINLINE LANGUAGE_RUNTIME_EXPORT void
_language_runtime_on_report(uintptr_t flags, const char *message,
                         RuntimeErrorDetails *details) {
  // Do nothing. This function is meant to be used by the debugger.

  // The following is necessary to avoid calls from being optimized out.
  asm volatile("" // Do nothing.
               : // Output list, empty.
               : "r" (flags), "r" (message), "r" (details) // Input list.
               : // Clobber list, empty.
               );
}

void language::_language_reportToDebugger(uintptr_t flags, const char *message,
                                    RuntimeErrorDetails *details) {
  _language_runtime_on_report(flags, message, details);
}

bool language::_language_reportFatalErrorsToDebugger = true;

bool language::_language_shouldReportFatalErrorsToDebugger() {
  return _language_reportFatalErrorsToDebugger;
}

/// Report a fatal error to system console, stderr, and crash logs.
/// Does not crash by itself.
void language::language_reportError(uint32_t flags,
                              const char *message) {
#if defined(__APPLE__) && NDEBUG
  flags &= ~FatalErrorFlags::ReportBacktrace;
#elif LANGUAGE_ENABLE_BACKTRACING
  // Disable fatalError backtraces if the backtracer is enabled
  if (runtime::backtrace::_language_backtrace_isEnabled()) {
    flags &= ~FatalErrorFlags::ReportBacktrace;
  }
#endif

  reportNow(flags, message);
  reportOnCrash(flags, message);
}

// Report a fatal error to system console, stderr, and crash logs, then abort.
LANGUAGE_NORETURN void language::fatalErrorv(uint32_t flags, const char *format,
                                       va_list args) {
  char *log;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
  language_vasprintf(&log, format, args);
#pragma GCC diagnostic pop

  language_reportError(flags, log);
  abort();
}

// Report a fatal error to system console, stderr, and crash logs, then abort.
LANGUAGE_NORETURN void language::fatalError(uint32_t flags, const char *format, ...) {
  va_list args;
  va_start(args, format);

  fatalErrorv(flags, format, args);
}

// Report a warning to system console and stderr.
void
language::warningv(uint32_t flags, const char *format, va_list args)
{
  char *log;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
  language_vasprintf(&log, format, args);
#pragma GCC diagnostic pop

  reportNow(flags, log);

  free(log);
}

// Report a warning to system console and stderr.
void
language::warning(uint32_t flags, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  warningv(flags, format, args);
}

/// Report a warning to the system console and stderr.  This is exported,
/// unlike the language::warning() function above.
void language::language_reportWarning(uint32_t flags, const char *message) {
  warning(flags, "%s", message);
}

#if !defined(LANGUAGE_HAVE_CRASHREPORTERCLIENT)
std::atomic<const char *> *language::language_getFatalErrorMessageBuffer() {
  return &kFatalErrorMessage;
}
#endif

// Crash when a deleted method is called by accident.
LANGUAGE_RUNTIME_EXPORT LANGUAGE_NORETURN void language_deletedMethodError() {
  language::fatalError(/* flags = */ 0,
                    "Fatal error: Call of deleted method\n");
}

// Crash due to a retain count overflow.
// FIXME: can't pass the object's address from InlineRefCounts without hacks
void language::language_abortRetainOverflow() {
  language::fatalError(FatalErrorFlags::ReportBacktrace,
                    "Fatal error: Object was retained too many times\n");
}

// Crash due to an unowned retain count overflow.
// FIXME: can't pass the object's address from InlineRefCounts without hacks
void language::language_abortUnownedRetainOverflow() {
  language::fatalError(FatalErrorFlags::ReportBacktrace,
                    "Fatal error: Object's unowned reference was retained too "
                    "many times\n");
}

// Crash due to a weak retain count overflow.
// FIXME: can't pass the object's address from InlineRefCounts without hacks
void language::language_abortWeakRetainOverflow() {
  language::fatalError(FatalErrorFlags::ReportBacktrace,
                    "Fatal error: Object's weak reference was retained too "
                    "many times\n");
}

// Crash due to retain of a dead unowned reference.
// FIXME: can't pass the object's address from InlineRefCounts without hacks
void language::language_abortRetainUnowned(const void *object) {
  if (object) {
    language::fatalError(FatalErrorFlags::ReportBacktrace,
                      "Fatal error: Attempted to read an unowned reference but "
                      "object %p was already destroyed\n", object);
  } else {
    language::fatalError(FatalErrorFlags::ReportBacktrace,
                      "Fatal error: Attempted to read an unowned reference but "
                      "the object was already destroyed\n");
  }
}

/// Halt due to enabling an already enabled dynamic replacement().
void language::language_abortDynamicReplacementEnabling() {
  language::fatalError(FatalErrorFlags::ReportBacktrace,
                    "Fatal error: trying to enable a dynamic replacement "
                    "that is already enabled\n");
}

/// Halt due to disabling an already disabled dynamic replacement().
void language::language_abortDynamicReplacementDisabling() {
  language::fatalError(FatalErrorFlags::ReportBacktrace,
                    "Fatal error: trying to disable a dynamic replacement "
                    "that is already disabled\n");
}

/// Halt due to a failure to allocate memory.
void language::language_abortAllocationFailure(size_t size, size_t alignMask) {
  language::fatalError(FatalErrorFlags::ReportBacktrace,
                    "Fatal error: failed to allocate %zu bytes of memory with "
                    "alignment %zu\n", size, alignMask + 1);
}

/// Halt due to trying to use unicode data on platforms that don't have it.
void language::language_abortDisabledUnicodeSupport() {
  language::fatalError(FatalErrorFlags::ReportBacktrace,
                    "Unicode normalization data is disabled on this "
                    "platform\n");

}

#if defined(_WIN32)
// On Windows, exceptions may be swallowed in some cases and the
// process may not terminate as expected on crashes. For example,
// illegal instructions used by toolchain.trap. Disable the exception
// swallowing so that the error handling works as expected.
__attribute__((__constructor__))
static void ConfigureExceptionPolicy() {
  BOOL Suppress = FALSE;
  SetUserObjectInformationA(GetCurrentProcess(),
                            UOI_TIMERPROC_EXCEPTION_SUPPRESSION,
                            &Suppress, sizeof(Suppress));
}

#endif
