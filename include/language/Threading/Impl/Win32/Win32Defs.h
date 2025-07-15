//==--- Win32Defs.h - Windows API definitions ------------------ -*-C++ -*-===//
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
// We cannot include <windows.h> from the Threading headers because they get
// included all over the place and <windows.h> defines a large number of
// obnoxious macros.  Instead, this header declares *just* what we need.
//
// If you need <windows.h> in a file, please make sure to include it *before*
// this file, or you'll get errors about RTL_SRWLOCK.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_THREADING_IMPL_WIN32_DEFS_H
#define LANGUAGE_THREADING_IMPL_WIN32_DEFS_H

#define DECLSPEC_IMPORT __declspec(dllimport)
#define WINBASEAPI DECLSPEC_IMPORT
#define WINAPI __stdcall
#define NTAPI __stdcall

// <windows.h> #defines VOID rather than typedefing it(!)  Changing that
// to use a typedef instead isn't problematic later on, so let's do that.
#undef VOID

typedef void VOID, *PVOID;
typedef unsigned char BYTE;
typedef BYTE BOOLEAN;
typedef int BOOL;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned long ULONG;
#if defined(_WIN64)
typedef unsigned __int64 ULONG_PTR;
#else
typedef unsigned long ULONG_PTR;
#endif
typedef PVOID HANDLE;

typedef VOID(NTAPI *PFLS_CALLBACK_FUNCTION)(PVOID lpFlsData);

typedef struct _RTL_SRWLOCK *PRTL_SRWLOCK;
typedef PRTL_SRWLOCK PSRWLOCK;

typedef struct _RTL_CONDITION_VARIABLE *PRTL_CONDITION_VARIABLE;
typedef PRTL_CONDITION_VARIABLE PCONDITION_VARIABLE;

typedef struct _RTL_CRITICAL_SECTION_DEBUG *PRTL_CRITICAL_SECTION_DEBUG;
typedef struct _RTL_CRITICAL_SECTION *PRTL_CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

// These have to be #defines, to avoid problems with <windows.h>
#define RTL_SRWLOCK_INIT {0}
#define SRWLOCK_INIT RTL_SRWLOCK_INIT
#define FLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)

#define RTL_CONDITION_VARIABLE_INIT {0}
#define CONDITION_VARIABLE_INIT RTL_CONDITION_VARIABLE_INIT

#define RTL_CONDITION_VARIABLE_LOCKMODE_SHARED 0x1
#define CONDITION_VARIABLE_LOCKMODE_SHARED RTL_CONDITION_VARIABLE_LOCKMODE_SHARED

#define INFINITE 0xFFFFFFFF // Infinite timeout

extern "C" {
WINBASEAPI DWORD WINAPI GetCurrentThreadId(VOID);

WINBASEAPI VOID WINAPI InitializeSRWLock(PSRWLOCK SRWLock);
WINBASEAPI VOID WINAPI ReleaseSRWLockExclusive(PSRWLOCK SRWLock);
WINBASEAPI VOID WINAPI AcquireSRWLockExclusive(PSRWLOCK SRWLock);
WINBASEAPI BOOLEAN WINAPI TryAcquireSRWLockExclusive(PSRWLOCK SRWLock);

WINBASEAPI VOID WINAPI InitializeConditionVariable(
  PCONDITION_VARIABLE ConditionVariable
);
WINBASEAPI VOID WINAPI WakeConditionVariable(
  PCONDITION_VARIABLE ConditionVariable
);
WINBASEAPI VOID WINAPI WakeAllConditionVariable(
  PCONDITION_VARIABLE ConditionVariable
);
WINBASEAPI BOOL WINAPI SleepConditionVariableSRW(
  PCONDITION_VARIABLE ConditionVariable,
  PSRWLOCK SRWLock,
  DWORD dwMilliseconds,
  ULONG Flags
);

WINBASEAPI VOID WINAPI InitializeCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
);
WINBASEAPI VOID WINAPI DeleteCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
);
WINBASEAPI VOID WINAPI EnterCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
);
WINBASEAPI VOID WINAPI LeaveCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
);

WINBASEAPI DWORD WINAPI FlsAlloc(PFLS_CALLBACK_FUNCTION lpCallback);
WINBASEAPI PVOID WINAPI FlsGetValue(DWORD dwFlsIndex);
WINBASEAPI BOOL WINAPI FlsSetValue(DWORD dwFlsIndex, PVOID lpFlsData);
WINBASEAPI BOOL WINAPI FlsFree(DWORD dwFlsIndex);
}

namespace language {
namespace threading_impl {

// We do this because we can't declare _RTL_SRWLOCK here in case someone
// later includes <windows.h>
struct LANGUAGE_SRWLOCK {
  PVOID Ptr;
};

typedef LANGUAGE_SRWLOCK *PLANGUAGE_SRWLOCK;

inline VOID InitializeSRWLock(PLANGUAGE_SRWLOCK SRWLock) {
  ::InitializeSRWLock(reinterpret_cast<PSRWLOCK>(SRWLock));
}
inline VOID ReleaseSRWLockExclusive(PLANGUAGE_SRWLOCK SRWLock) {
  ::ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(SRWLock));
}
inline VOID AcquireSRWLockExclusive(PLANGUAGE_SRWLOCK SRWLock) {
  ::AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(SRWLock));
}
inline BOOLEAN TryAcquireSRWLockExclusive(PLANGUAGE_SRWLOCK SRWLock) {
  return ::TryAcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(SRWLock));
}

// Similarly we have the same problem with _RTL_CONDITION_VARIABLE
struct LANGUAGE_CONDITION_VARIABLE {
  PVOID Ptr;
};

typedef LANGUAGE_CONDITION_VARIABLE *PLANGUAGE_CONDITION_VARIABLE;

inline VOID InitializeConditionVariable(PLANGUAGE_CONDITION_VARIABLE CondVar) {
  ::InitializeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(CondVar));
}
inline VOID WakeConditionVariable(PLANGUAGE_CONDITION_VARIABLE CondVar) {
  ::WakeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(CondVar));
}
inline VOID WakeAllConditionVariable(PLANGUAGE_CONDITION_VARIABLE CondVar) {
  ::WakeAllConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(CondVar));
}
inline BOOL SleepConditionVariableSRW(PLANGUAGE_CONDITION_VARIABLE CondVar,
                                      PLANGUAGE_SRWLOCK SRWLock,
                                      DWORD dwMilliseconds,
                                      ULONG Flags) {
  return ::SleepConditionVariableSRW(
    reinterpret_cast<PCONDITION_VARIABLE>(CondVar),
    reinterpret_cast<PSRWLOCK>(SRWLock),
    dwMilliseconds,
    Flags);
}

// And with CRITICAL_SECTION
#pragma pack(push, 8)
typedef struct LANGUAGE_CRITICAL_SECTION {
  PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
  LONG LockCount;
  LONG RecursionCount;
  HANDLE OwningThread;
  HANDLE LockSemaphore;
  ULONG_PTR SpinCount;
} LANGUAGE_CRITICAL_SECTION, *PLANGUAGE_CRITICAL_SECTION;
#pragma pack(pop)

inline VOID InitializeCriticalSection(PLANGUAGE_CRITICAL_SECTION CritSec) {
  ::InitializeCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(CritSec));
}

inline VOID DeleteCriticalSection(PLANGUAGE_CRITICAL_SECTION CritSec) {
  ::DeleteCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(CritSec));
}

inline VOID EnterCriticalSection(PLANGUAGE_CRITICAL_SECTION CritSec) {
  ::EnterCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(CritSec));
}

inline VOID LeaveCriticalSection(PLANGUAGE_CRITICAL_SECTION CritSec) {
  ::LeaveCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(CritSec));
}

} // namespace threading_impl
} // namespace language

#endif // LANGUAGE_THREADING_IMPL_WIN32_DEFS_H
