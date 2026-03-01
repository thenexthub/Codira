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

/**
 * @file
 *
 * This file implements macro evaluation related apis for macro.
 */

#include "Codira/Macro/MacroEvaluation.h"
#include "Codira/Macro/TokenSerialization.h"

using namespace Codira;
using namespace AST;
using namespace InvokeRuntime;

namespace {
using TaskFunc = void* (*)(void*);
using InvokeNewTaskFromC = void* (*)(TaskFunc, void*);
using ReleaseHandleFromC = void (*)(void*);
/*
 * For compiled macro, evaluate macrocall by invoking method in runtime.
 */
struct ResetNotify {
    void* cFunc;
    void* param;
    ResetNotify(void* notifyFunc, void* notifyParam) : cFunc(notifyFunc), param(notifyParam)
    {
    }
};

struct MacroInvoke {
public:
    MacroCall* macCall;
    std::mutex mtx;
    std::condition_variable cv;
    bool isDataReady{false};
    explicit MacroInvoke(MacroCall* mc) : macCall(mc)
    {
    }
};

void ResetVariableFinishNotify(void* mi)
{
    auto mInvoke = static_cast<MacroInvoke*>(mi);
    // here, we need to wait main thread get the mutex first, and then cv.wait unlock the mutex,
    // then we get the lock and notify, to make sure that wait in main thread is before notify.
    std::unique_lock<std::mutex> lck(mInvoke->mtx);
    mInvoke->isDataReady = true;
    mInvoke->cv.notify_one();
}

HANDLE InvokeMacroFunc(HANDLE mc)
{
    auto pMacCall = static_cast<MacroCall*>(mc);
    auto pInvocation = pMacCall->GetInvocation();
    auto attrTksBytes = TokenSerialization::GetTokensBytes(pInvocation->attrs);
    auto inputTksBytes = TokenSerialization::GetTokensBytes(pInvocation->args);

    uint8_t* retBuffer{nullptr};
    if (pInvocation->hasAttr) {
        auto attrMacroFunc = reinterpret_cast<AttrFuncPtrT>(pMacCall->invokeFunc);
        retBuffer = attrMacroFunc(attrTksBytes.data(), static_cast<int64_t>(attrTksBytes.size()),
            inputTksBytes.data(), static_cast<int64_t>(inputTksBytes.size()), pMacCall);
    } else {
        auto commonMacroFunc = reinterpret_cast<CommonFuncPtrT>(pMacCall->invokeFunc);
        retBuffer = commonMacroFunc(inputTksBytes.data(), static_cast<int64_t>(inputTksBytes.size()), pMacCall);
    }
    pInvocation->newTokens = TokenSerialization::GetTokensFromBytes(retBuffer);
    if (retBuffer != nullptr) {
        free(retBuffer);
        retBuffer = nullptr;
    }
    pMacCall->isDataReady = true;
    return nullptr;
}

HANDLE InvokeMacro(HANDLE mi)
{
    // For serial macro expansion.
    auto mInvoke = static_cast<MacroInvoke*>(mi);
    // here, we need to wait main thread get the mutex first, and then cv.wait unlock the mutex,
    // then we get the lock and notify, to make sure that wait in main thread is before notify.
    std::unique_lock<std::mutex> lck(mInvoke->mtx);
    (void)InvokeMacroFunc(mInvoke->macCall);
    mInvoke->cv.notify_one();
    return nullptr;
}
} // namespace

void MacroEvaluation::CollectMacroLibs()
{
    if (!ci->invocation.globalOptions.macroLib.empty()) {
        // Support to use --macro-lib option.
        // The following code will be deleted.
        auto dynFiles = GetMacroDefDynamicFiles();
        for (auto& dyfile : dynFiles) {
            auto handle = InvokeRuntime::OpenSymbolTable(dyfile);
            if (handle == nullptr) {
                (void)ci->diag.Diagnose(DiagKind::can_not_open_macro_library, dyfile);
                return;
            }
            InvokeRuntime::SetOpenedLibHandles(handle);
        }
    }
}

void MacroEvaluation::EvaluateWithRuntime(MacroCall& macCall)
{
    if (useChildProcess) {
        // For lsp, send task to child process for expanding macrocall.
        if (SendMacroCallTask(macCall)) {
            if (!enableParallelMacro) {
                // If send task successful and in serial macro mode, wait for eval result from child process.
                WaitMacroCallEvalResult(macCall);
            }
        }
        return;
    }
    auto invokeMethodFuncFromC = reinterpret_cast<InvokeNewTaskFromC>(RuntimeInit::GetInstance().runtimeMethodFunc);
    if (enableParallelMacro) {
        // For parallel macro expansion.
        macCall.coroutineHandle = invokeMethodFuncFromC(InvokeMacroFunc, &macCall);
        CODEC_NULLPTR_CHECK(macCall.coroutineHandle);
    } else {
        // For serial macro expansion.
        MacroInvoke mi(&macCall);
        // here we only have one main-thead, so we wouldn't have multi-thread change the attrTksBytes.
        // make sure that we get the mutex before invoke thread.
        std::unique_lock<std::mutex> lck(mi.mtx);
        macCall.coroutineHandle = invokeMethodFuncFromC(InvokeMacro, &mi);
        CODEC_NULLPTR_CHECK(macCall.coroutineHandle);
        mi.cv.wait(lck, [&macCall] { return macCall.isDataReady; });
        ReleaseThreadHandle(macCall);
    }
}

void MacroEvaluation::InitGlobalVariable()
{
    // Find RunCODETask and ReleaseHandle from runtime dylib.
    auto invokeMethodFuncFromC = reinterpret_cast<InvokeNewTaskFromC>(RuntimeInit::GetInstance().runtimeMethodFunc);
    auto invokeReleaseHandleFromC = reinterpret_cast<ReleaseHandleFromC>(RuntimeInit::GetInstance().runtimeReleaseFunc);
    for (auto& [mpkgname, hasInit] : usedMacroPkgs) {
        if (hasInit) {
            continue;
        }
        // method: <PackageName>_global_init$_reset / <ModuleName>_<PackageName>_global_init$_reset
        auto macrodef = mpkgname;
        auto method = MANGLE_CODIRA_PREFIX + MANGLE_GLOBAL_PACKAGE_INIT_PREFIX + MangleUtils::GetOptPkgName(macrodef) +
            SPECIAL_NAME_FOR_INIT_RESET_FUNCTION + MANGLE_FUNC_PARAM_TYPE_PREFIX + MANGLE_VOID_TY_SUFFIX;
        bool findGlobalFunc = false;
        for (auto& handle : InvokeRuntime::GetOpenedLibHandles()) {
            auto initGlobalFunc = reinterpret_cast<InitGlobalFuncPtr>(GetMethod(handle, method.c_str()));
            if (!initGlobalFunc) {
                continue;
            }
            MacroInvoke mi(nullptr);
            // Get the mutex before invoke thread.
            std::unique_lock<std::mutex> lck(mi.mtx);
            auto resetNotify = std::make_unique<ResetNotify>(reinterpret_cast<void*>(ResetVariableFinishNotify), &mi);
            auto coroutineHandle = invokeMethodFuncFromC(initGlobalFunc, resetNotify.get());
            CODEC_NULLPTR_CHECK(coroutineHandle);
            // Unlock the mutex after invoke thread notify.
            mi.cv.wait(lck, [&mi] { return mi.isDataReady; });
            invokeReleaseHandleFromC(coroutineHandle);
            findGlobalFunc = true;
            usedMacroPkgs.at(mpkgname) = true;
            break;
        }
        // Can not find global reset func.
        if (!findGlobalFunc) {
            Warningln("could not find global reset method in macro dylib: ", method);
        }
    }
}

void MacroEvaluation::ReleaseThreadHandle(MacroCall& macCall)
{
    auto invokeReleaseHandle =
        reinterpret_cast<ReleaseHandleFromC>(RuntimeInit::GetInstance().runtimeReleaseFunc);
    invokeReleaseHandle(macCall.coroutineHandle);
}
