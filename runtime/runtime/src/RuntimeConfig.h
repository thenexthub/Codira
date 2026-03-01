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


#ifndef MRT_RUNTIME_CONFIG_H
#define MRT_RUNTIME_CONFIG_H

#include <cstddef>
#include <cstdint>

#include "Codira.h"
#include "os/Loader.h"

#ifdef __cplusplus
namespace MapleRuntime {
extern "C" {
#endif

struct BinLoadApi {
    void* (*binLoad)(const char*);
    int (*binUnload)(void*);
    int (*getBinaryInfoFromAddress)(const void*, Os::Loader::BinaryInfo*);
    void* (*getBinHandle)(const char*);
    void* (*findSymbol)(void*, const char* symbolName);
    BinLoadApi()
        : binLoad(nullptr), binUnload(nullptr), getBinaryInfoFromAddress(nullptr), getBinHandle(nullptr),
          findSymbol(nullptr) {}
};

MRT_EXPORT uintptr_t MRT_StopGCWork();
MRT_EXPORT uintptr_t MRT_GetThreadLocalData();
MRT_EXPORT void MRT_VisitorCaller(void* argPtr, void* handle);
MRT_EXPORT void MRT_DumpLog(const char* message);
uintptr_t MRT_GetSafepointProtectedPage();
uintptr_t MRT_CreateMutator();
uintptr_t MRT_TransitMutatorToExit();
#if defined(__APPLE__) && defined(__aarch64__)
void* ExecuteCodiraStub(void*, void*, void*, void*, void*, void*);
void* ApplyCodiraMethodStub(void*, void*, void*, void*, void*);
float ApplyCodiraMethodStubFloat32(void*, void*, void*, void*);
double ApplyCodiraMethodStubFloat64(void*, void*, void*, void*);
#ifdef __OHOS__
MRT_EXPORT void* CODE_MRT_ARKTS_CreateEngineStub();
#endif
#else
void* ExecuteCodiraStub(...);
bool InitCODELibraryStub(...);
void* ApplyCodiraMethodStub(...);
float ApplyCodiraMethodStubFloat32(...);
double ApplyCodiraMethodStubFloat64(...);
#endif
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
void MRT_DumpAllStackTrace();
#endif

MRT_EXPORT bool MRT_CODELibInit(const char* libName);
MRT_EXPORT int LoadCODELibraryWithInit(const char* libName);
MRT_EXPORT int MRT_IsLoadedFile(const char* libName);
#ifdef __cplusplus
}
}
#endif
#endif // MRT_RUNTIME_CONFIG_H
