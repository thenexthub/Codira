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


#include "WinModuleManager.h"

#include <dbghelp.h>
#include <string>
#include <unordered_set>
#include <windows.h>
#include <winternl.h>

namespace MapleRuntime {
struct LdrDataTableEntry {
    LIST_ENTRY inLoadOrderLinks;
    LIST_ENTRY inMemoryOrderLinks;
    LIST_ENTRY inInitializationOrderLinks;
    PVOID dllBase;
    PVOID entryPoint;
    DWORD sizeOfImage;
    UNICODE_STRING fullDllName;
    UNICODE_STRING baseDllName;
    DWORD flags;
    WORD loadCount;
    WORD tlsIndex;
    LIST_ENTRY hashLinks;
    PVOID sectionPointer;
    DWORD checkSum;
    DWORD timeDateStamp;
    PVOID loadedImports;
    PVOID entryPointActivationContext;
    PVOID patchInformation;
};

RuntimeFunction* WinModule::GetRuntimeFunction(Uptr pc) const
{
    uint32_t start = 0;
    uint32_t end = funcTableCount - 1;
    uint32_t mid = 0;
    while (start <= end) {
        mid = (start + end) / 2; // 2: half of (start + end)
        if (IsInRuntimeFunc(mid, pc)) {
            return &(funcTable[mid]);
        } else if (pc - imageBaseStart > funcTable[mid].endAddress) {
            start = mid + 1;
        } else {
            end = mid - 1;
        }
    }
    return nullptr;
}

WinModule* WinModuleManager::GetWinModuleByPc(Uptr pc) const
{
    for (auto module : winModules) {
        if (module->IsInModule(pc)) {
            return module;
        }
    }
    return nullptr;
}

WinModule* WinModuleManager::GetWinModuleByName(CString name) const
{
    for (auto module : winModules) {
        if (module->GetModuleName() == name) {
            return module;
        }
    }
    return nullptr;
}

void WinModuleManager::Init() { ReadWinModuleAtInit(); }

void WinModuleManager::ReadWinModuleAtInit()
{
    // get windows PEB(Process Environment Block)
    PPEB ppeb = (PPEB)__readgsqword(0x60); // 60: gs:0x60 is the address of PEB
    PLIST_ENTRY head = ppeb->Ldr->InMemoryOrderModuleList.Flink;
    PLIST_ENTRY iterator = head;
    do {
        LdrDataTableEntry* ldrDataEntry =
            (LdrDataTableEntry*)CONTAINING_RECORD(iterator, LdrDataTableEntry, inMemoryOrderLinks);
        // get moduleName
        if (ldrDataEntry->baseDllName.Buffer == nullptr) {
            iterator = iterator->Flink;
            continue;
        }
        std::wstring wstr(ldrDataEntry->baseDllName.Buffer);
        std::string moduleName(wstr.begin(), wstr.end());

        // exclude nativeLibNames
        if (!moduleName.empty() && nativeLibNames.count(moduleName) == 0) {
            ULONG funcTableSize = 0;
            RuntimeFunction* funcTable = reinterpret_cast<RuntimeFunction*>(ImageDirectoryEntryToData(
                ldrDataEntry->dllBase, true, IMAGE_DIRECTORY_ENTRY_EXCEPTION, &funcTableSize));
            uint32_t funcTableCount = funcTableSize / sizeof(RuntimeFunction);

            Uptr imageBaseStart = reinterpret_cast<Uptr>(ldrDataEntry->dllBase);
            Uptr imageBaseEnd = imageBaseStart + ldrDataEntry->sizeOfImage;
            WinModule* winModule = new (std::nothrow)
                WinModule(imageBaseStart, imageBaseEnd, funcTable, funcTableCount, moduleName.c_str());
            if (UNLIKELY(winModule == nullptr)) {
                LOG(RTLOG_FATAL, "new WinModule failed.");
            }
            winModules.emplace(winModule);
        }

        iterator = iterator->Flink;
    } while (iterator != head);
}

void WinModuleManager::ReadModuleInfo(HMODULE* moduleHandlers, int moduleHandlersCapacity)
{
    for (int i = 0; i < moduleHandlersCapacity; i++) {
        // Get image name
        TCHAR moduleFullName[MAX_PATH];
        GetModuleFileNameA(moduleHandlers[i], moduleFullName, MAX_PATH);
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];
        _splitpath_s(moduleFullName, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
        std::string moduleFName = std::string(drive) + std::string(dir) + std::string(fname) + std::string(ext);
        std::string moduleName = std::string(fname) + std::string(ext);
        if (nativeLibNames.count(moduleName) != 0) {
            continue;
        }

        MODULEINFO module;
        // Get image address
        GetModuleInformation(GetCurrentProcess(), moduleHandlers[i], &module, sizeof(module));
        Uptr imageBaseStart = reinterpret_cast<Uptr>(module.lpBaseOfDll);
        Uptr imageBaseEnd = reinterpret_cast<Uptr>(module.lpBaseOfDll) + module.SizeOfImage;

        ULONG funcTableSize = 0;
        RuntimeFunction* funcTable = reinterpret_cast<RuntimeFunction*>(
            ImageDirectoryEntryToData((PVOID)imageBaseStart, true, IMAGE_DIRECTORY_ENTRY_EXCEPTION, &funcTableSize));
        uint32_t funcTableCount = funcTableSize / sizeof(RuntimeFunction);

        WinModule* winModule =
            new (std::nothrow) WinModule(imageBaseStart, imageBaseEnd, funcTable, funcTableCount, moduleFName.c_str());
        if (UNLIKELY(winModule == nullptr)) {
            LOG(RTLOG_FATAL, "new WinModule failed.");
        }
        winModules.emplace(winModule);
    }
}

void WinModuleManager::ReadWinModuleAtRunning()
{
    // It is hard to predict how many modules there will be in loading, here we assume 50 modules at the beginning.
    int moduleHandlerCapacity = 50;
    HMODULE moduleHandlers[moduleHandlerCapacity];
    DWORD neededModulesLength;
    HANDLE curProcess = GetCurrentProcess();
    EnumProcessModules(curProcess, moduleHandlers, sizeof(moduleHandlers), &neededModulesLength);
    int moduleNum = neededModulesLength / sizeof(HMODULE);
    if (moduleNum <= moduleHandlerCapacity) {
        ReadModuleInfo(moduleHandlers, moduleNum);
        return;
    }
    // require capacity expansion.
    HMODULE expandedModuleHandlers[moduleNum];
    EnumProcessModules(curProcess, expandedModuleHandlers, sizeof(expandedModuleHandlers), &neededModulesLength);
    ReadModuleInfo(expandedModuleHandlers, moduleNum);
}

void WinModuleManager::Fini() const
{
    for (auto module : winModules) {
        delete module;
        module = nullptr;
    }
}

} // namespace MapleRuntime
