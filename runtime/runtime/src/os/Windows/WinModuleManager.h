/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


#ifndef MRT_WIN_MODULE_MANAGER_H
#define MRT_WIN_MODULE_MANAGER_H

#include <windows.h>
#include <psapi.h>
#include <unordered_set>

#include "Base/CString.h"
#include "Base/Types.h"
#include "Common/StackType.h"

namespace MapleRuntime {
struct RuntimeFunction {
    uint32_t startAddress;
    uint32_t endAddress;
    uint32_t unwindInfoOffset;
};

// windows module means dll or exe loaded in memory.
class WinModule {
public:
    WinModule(Uptr imageStart, Uptr imageEnd, RuntimeFunction* funcTable, uint32_t fTableCount, const char* name)
        : imageBaseStart(imageStart), imageBaseEnd(imageEnd), funcTable(funcTable), funcTableCount(fTableCount),
          moduleName(name) {}
    ~WinModule() = default;

    bool IsInModule(Uptr pc) const { return pc >= imageBaseStart && pc <= imageBaseEnd; }

    bool IsInRuntimeFunc(uint32_t index, Uptr pc) const
    {
        Uptr rvaPc = pc - imageBaseStart;
        return rvaPc >= funcTable[index].startAddress && rvaPc <= funcTable[index].endAddress;
    }

    Uptr GetImageBaseStart() const { return imageBaseStart; }

    Uptr GetImageBaseEnd() const { return imageBaseEnd; }

    CString GetModuleName() const { return moduleName; }

    RuntimeFunction* GetRuntimeFunction(Uptr rip) const;

private:
    Uptr imageBaseStart;
    Uptr imageBaseEnd;
    RuntimeFunction* funcTable;
    uint32_t funcTableCount;
    CString moduleName;
};

struct WinModuleHash {
    std::size_t operator()(const WinModule* module) const
    {
        return std::hash<std::string>()(module->GetModuleName().Str());
    }
};

struct WinModuleCmp {
    bool operator()(const WinModule* lhs, const WinModule* rhs) const
    {
        return lhs->GetModuleName() == rhs->GetModuleName();
    }
};

class WinModuleManager {
public:
    WinModuleManager() = default;
    ~WinModuleManager() = default;

    void Init();
    void Fini() const;

    WinModule* GetWinModuleByPc(Uptr pc) const;
    WinModule* GetWinModuleByName(CString name) const;

    void ReadWinModuleAtInit();
    void ReadWinModuleAtRunning();

private:
    void ReadModuleInfo(HMODULE* moduleHandler, int capacity);
    std::unordered_set<WinModule*, WinModuleHash, WinModuleCmp> winModules;
    std::unordered_set<std::string> nativeLibNames{
        "ntdll.dll",       "KERNEL32.DLL",        "KERNELBASE.dll", "msvcrt.dll",           "libgcc_s_seh-1.dll",
        "libstdc++-6.dll", "libwinpthread-1.dll", "ucrtbase.dll",   "dbghelp.dll",          "libssp-0.dll",
        "ADVAPI32.dll",    "sechost.dll",         "RPCRT4.dll",     "libsecurec.dll",       "CRYPTSP.dll",
        "rsaenh.dll",      "bcrypt.dll",          "CRYPTBASE.dll",  "bcryptPrimitives.dll", "SYSFER.DLL"
    };
};
} // namespace MapleRuntime
#endif // MRT_WIN_MODULE_MANAGER_H
