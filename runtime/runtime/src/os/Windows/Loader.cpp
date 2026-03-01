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


#include "os/Loader.h"

#include <winerror.h>
#include <errhandlingapi.h>
#include <libloaderapi.h>
#include <memoryapi.h>

#include "Base/Log.h"

namespace MapleRuntime {
namespace Os {
static HMODULE GetModuleHandler(const void* address)
{
    if (address == nullptr) {
        LOG(RTLOG_ERROR, "address is nullptr.");
        return nullptr;
    }

    HMODULE moduleHandler = nullptr;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCSTR>(address), &moduleHandler)) {
        LOG(RTLOG_ERROR, "get module handler from address failed. ErrorCode is %u", GetLastError());
        return nullptr;
    }
    return moduleHandler;
}

void* Loader::LoadBinaryFile(const char* path)
{
    void* handler = LoadLibraryA(path);
    return handler;
}

int Loader::UnloadBinaryFile(void* handler)
{
    if (handler == nullptr) {
        return -1;
    }
    int ret = FreeLibrary(static_cast<HMODULE>(handler));
    return (ret != 0) ? 0 : -1;
}

void* Loader::FindSymbol(void* handler, const char* symbolName)
{
    if (symbolName == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handler), symbolName));
}

void Loader::GetBinaryFilePath(const void* address, BinaryInfo* binInfo)
{
    HMODULE moduleHandler = GetModuleHandler(address);
    if (moduleHandler == nullptr) {
        return;
    }

    char fileName[MAX_PATH];
    DWORD retNameSize = 0;
    retNameSize = GetModuleFileNameA(moduleHandler, reinterpret_cast<LPSTR>(fileName), sizeof(fileName));
    if (retNameSize == 0 || ((retNameSize == sizeof(fileName)) && GetLastError() == ERROR_SUCCESS)) {
        LOG(RTLOG_ERROR, "getModuleFileName failed.");
        return;
    }
    binInfo->filePathName = FixedCString(fileName);
}

void Loader::GetSymbolName(const void* address, BinaryInfo* binInfo)
{
    HMODULE moduleHandler = GetModuleHandler(address);
    if (moduleHandler == nullptr) {
        return;
    }

    // Get IMAGE_EXPORT_DIRECTORY in PE file.
    IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(moduleHandler);
    IMAGE_NT_HEADERS* ntHeaders =
        reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<BYTE*>(dosHeader) + dosHeader->e_lfanew);
    IMAGE_OPTIONAL_HEADER* optionalHeader = &(ntHeaders->OptionalHeader);
    if (optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0 ||
        optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0) {
        return;
    }
    IMAGE_EXPORT_DIRECTORY* ied = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
        reinterpret_cast<BYTE*>(moduleHandler) +
        (optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress));
    // By traversing IED, we get the address and name of each exported function. And then by comparing the address, get
    // the name of symbol whose definition overlaps addr.
    DWORD* AddressesOffsetsTable =
        reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(moduleHandler) + ied->AddressOfFunctions);
    int funcTableNums = ied->NumberOfFunctions;
    void* candidateAddr = nullptr;
    int candidateIndex = -1;
    for (int i = 0; i < funcTableNums; i++) {
        // AddressesOffsetsTable table is not a sorted array.
        if (reinterpret_cast<void*>(reinterpret_cast<BYTE*>(moduleHandler) + AddressesOffsetsTable[i]) > address ||
            candidateAddr >=
                reinterpret_cast<void*>(reinterpret_cast<BYTE*>(moduleHandler) + AddressesOffsetsTable[i])) {
            continue;
        }
        candidateAddr = reinterpret_cast<void*>(reinterpret_cast<BYTE*>(moduleHandler) + AddressesOffsetsTable[i]);
        candidateIndex = i;
    }
    if (candidateIndex == -1) {
        LOG(RTLOG_ERROR, "not found any symbol in file.");
        return;
    }
    DWORD* addressOfNamesTable = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(moduleHandler) + ied->AddressOfNames);
    USHORT* addressOfNameOrdinalsTable =
        reinterpret_cast<USHORT*>(reinterpret_cast<BYTE*>(moduleHandler) + ied->AddressOfNameOrdinals);
    int nameTableNums = ied->NumberOfNames;
    for (int i = 0; i < nameTableNums; i++) {
        if (addressOfNameOrdinalsTable[i] == candidateIndex) {
            binInfo->symbolName =
                FixedCString(reinterpret_cast<char*>(reinterpret_cast<BYTE*>(moduleHandler) + addressOfNamesTable[i]));
            return;
        }
    }
    LOG(RTLOG_ERROR, "not found any symbol in file.");
}

bool Loader::IsValidAddress(const void* address)
{
    MEMORY_BASIC_INFORMATION memoryInfo;
    if (VirtualQuery(address, &memoryInfo, sizeof(memoryInfo))) {
        return ((memoryInfo.AllocationBase != nullptr) && memoryInfo.AllocationProtect != 0 &&
                memoryInfo.AllocationProtect != PAGE_NOACCESS) ?
            true :
            false;
    }
    return false;
}

int Loader::GetBinaryInfoFromAddress(const void* address, BinaryInfo* binInfo)
{
    if (address == nullptr) {
        return 0;
    }
    if (!IsValidAddress(address)) {
        return 0;
    }
    GetBinaryFilePath(address, binInfo);
    GetSymbolName(address, binInfo);
    return (binInfo->filePathName.IsEmpty()) ? 0 : 1;
}
} // namespace Os
} // namespace MapleRuntime
