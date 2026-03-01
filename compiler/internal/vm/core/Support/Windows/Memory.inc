//===- Win32/Memory.cpp - Win32 Memory Implementation -----------*- C++ -*-===//
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
//
// This file provides the Win32 specific implementation of various Memory
// management utilities
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/DataTypes.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/Process.h"
#include "vm/core/Support/WindowsError.h"

// The Windows.h header must be the last one included.
#include "vm/core/Support/Windows/WindowsSupport.h"

static DWORD getWindowsProtectionFlags(unsigned Flags) {
  switch (Flags & toolchain::sys::Memory::MF_RWE_MASK) {
  // Contrary to what you might expect, the Windows page protection flags
  // are not a bitwise combination of RWX values
  case toolchain::sys::Memory::MF_READ:
    return PAGE_READONLY;
  case toolchain::sys::Memory::MF_WRITE:
    // Note: PAGE_WRITE is not supported by VirtualProtect
    return PAGE_READWRITE;
  case toolchain::sys::Memory::MF_READ | toolchain::sys::Memory::MF_WRITE:
    return PAGE_READWRITE;
  case toolchain::sys::Memory::MF_READ | toolchain::sys::Memory::MF_EXEC:
    return PAGE_EXECUTE_READ;
  case toolchain::sys::Memory::MF_READ | toolchain::sys::Memory::MF_WRITE |
      toolchain::sys::Memory::MF_EXEC:
    return PAGE_EXECUTE_READWRITE;
  case toolchain::sys::Memory::MF_EXEC:
    return PAGE_EXECUTE;
  default:
    llvm_unreachable("Illegal memory protection flag specified!");
  }
  // Provide a default return value as required by some compilers.
  return PAGE_NOACCESS;
}

// While we'd be happy to allocate single pages, the Windows allocation
// granularity may be larger than a single page (in practice, it is 64K)
// so mapping less than that will create an unreachable fragment of memory.
static size_t getAllocationGranularity() {
  SYSTEM_INFO Info;
  ::GetSystemInfo(&Info);
  if (Info.dwPageSize > Info.dwAllocationGranularity)
    return Info.dwPageSize;
  else
    return Info.dwAllocationGranularity;
}

// Large/huge memory pages need explicit process permissions in order to be
// used. See https://blogs.msdn.microsoft.com/oldnewthing/20110128-00/?p=11643
// Also large pages need to be manually enabled on your OS. If all this is
// sucessfull, we return the minimal large memory page size.
static size_t enableProcessLargePages() {
  HANDLE Token = 0;
  size_t LargePageMin = GetLargePageMinimum();
  if (LargePageMin)
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                     &Token);
  if (!Token)
    return 0;
  LUID Luid;
  if (!LookupPrivilegeValue(0, SE_LOCK_MEMORY_NAME, &Luid)) {
    CloseHandle(Token);
    return 0;
  }
  TOKEN_PRIVILEGES TP{};
  TP.PrivilegeCount = 1;
  TP.Privileges[0].Luid = Luid;
  TP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  if (!AdjustTokenPrivileges(Token, FALSE, &TP, 0, 0, 0)) {
    CloseHandle(Token);
    return 0;
  }
  DWORD E = GetLastError();
  CloseHandle(Token);
  if (E == ERROR_SUCCESS)
    return LargePageMin;
  return 0;
}

namespace vm::core {
namespace sys {

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only Win32 specific code
//===          and must not be UNIX code
//===----------------------------------------------------------------------===//

MemoryBlock Memory::allocateMappedMemory(size_t NumBytes,
                                         const MemoryBlock *const NearBlock,
                                         unsigned Flags, std::error_code &EC) {
  EC = std::error_code();
  if (NumBytes == 0)
    return MemoryBlock();

  static size_t DefaultGranularity = getAllocationGranularity();
  static size_t LargePageGranularity = enableProcessLargePages();

  DWORD AllocType = MEM_RESERVE | MEM_COMMIT;
  bool HugePages = false;
  size_t Granularity = DefaultGranularity;

  if ((Flags & MF_HUGE_HINT) && LargePageGranularity > 0) {
    AllocType |= MEM_LARGE_PAGES;
    HugePages = true;
    Granularity = LargePageGranularity;
  }

  size_t NumBlocks = (NumBytes + Granularity - 1) / Granularity;

  uintptr_t Start = NearBlock ? reinterpret_cast<uintptr_t>(NearBlock->base()) +
                                    NearBlock->allocatedSize()
                              : 0;

  // If the requested address is not aligned to the allocation granularity,
  // round up to get beyond NearBlock. VirtualAlloc would have rounded down.
  if (Start && Start % Granularity != 0)
    Start += Granularity - Start % Granularity;

  DWORD Protect = getWindowsProtectionFlags(Flags);

  size_t AllocSize = NumBlocks * Granularity;
  void *PA = ::VirtualAlloc(reinterpret_cast<void *>(Start), AllocSize,
                            AllocType, Protect);
  if (PA == NULL) {
    if (NearBlock || HugePages) {
      // Try again without the NearBlock hint and without large memory pages
      return allocateMappedMemory(NumBytes, NULL, Flags & ~MF_HUGE_HINT, EC);
    }
    EC = mapWindowsError(::GetLastError());
    return MemoryBlock();
  }

  MemoryBlock Result;
  Result.Address = PA;
  Result.AllocatedSize = AllocSize;
  Result.Flags = (Flags & ~MF_HUGE_HINT) | (HugePages ? MF_HUGE_HINT : 0);

  if (Flags & MF_EXEC)
    Memory::InvalidateInstructionCache(Result.Address, AllocSize);

  return Result;
}

std::error_code Memory::releaseMappedMemory(MemoryBlock &M) {
  if (M.Address == 0 || M.AllocatedSize == 0)
    return std::error_code();

  if (!VirtualFree(M.Address, 0, MEM_RELEASE))
    return mapWindowsError(::GetLastError());

  M.Address = 0;
  M.AllocatedSize = 0;

  return std::error_code();
}

std::error_code Memory::protectMappedMemory(const MemoryBlock &M,
                                            unsigned Flags) {
  if (M.Address == 0 || M.AllocatedSize == 0)
    return std::error_code();

  DWORD Protect = getWindowsProtectionFlags(Flags);

  DWORD OldFlags;
  if (!VirtualProtect(M.Address, M.AllocatedSize, Protect, &OldFlags))
    return mapWindowsError(::GetLastError());

  if (Flags & MF_EXEC)
    Memory::InvalidateInstructionCache(M.Address, M.AllocatedSize);

  return std::error_code();
}

/// InvalidateInstructionCache - Before the JIT can run a block of code
/// that has been emitted it must invalidate the instruction cache on some
/// platforms.
void Memory::InvalidateInstructionCache(const void *Addr, size_t Len) {
  FlushInstructionCache(GetCurrentProcess(), Addr, Len);
}

} // namespace sys
} // namespace vm::core
