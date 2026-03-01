/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "backend/js_pt_hooks.h"
#include "agent/debugger_impl.h"
#include "../hybrid_step/debug_step_flags.h"

namespace panda::ecmascript::tooling {
void JSPtHooks::DebuggerStmt([[maybe_unused]] const JSPtLocation &location)
{
    LOG_DEBUGGER(DEBUG) << "JSPHooks: Debugger Statement";
    [[maybe_unused]] LocalScope scope(debugger_->vm_);
    debugger_->NotifyPaused({}, DEBUGGERSTMT);
}

void JSPtHooks::Breakpoint(const JSPtLocation &location)
{
    LOG_DEBUGGER(DEBUG) << "JSPtHooks: Breakpoint => " << location.GetMethodId() << ": "
                         << location.GetBytecodeOffset();

    [[maybe_unused]] LocalScope scope(debugger_->vm_);
    debugger_->NotifyPaused(location, OTHER);
}

void JSPtHooks::Exception([[maybe_unused]] const JSPtLocation &location)
{
    LOG_DEBUGGER(DEBUG) << "JSPtHooks: Exception";
    [[maybe_unused]] LocalScope scope(debugger_->vm_);

    debugger_->NotifyPaused({}, EXCEPTION);
}

bool JSPtHooks::SingleStep(const JSPtLocation &location)
{
    [[maybe_unused]] LocalScope scope(debugger_->vm_);

    // Break_on_start event will be the very first one
    // to be verified or notified
    if (UNLIKELY(firstTime_)) {
        firstTime_ = false;

        debugger_->NotifyPaused({}, BREAK_ON_START);
        return false;
    }

    // Do not set hybrid step flag when executing non-application pandafiles
    auto jsPandaFile = location.GetJsPandaFile();
    if (jsPandaFile != nullptr && debugger_->IsApplicationFile(DebuggerApi::GetJSPandaFileDesc(jsPandaFile).data())) {
        DebugStepFlags::Get().SetDyn2StatInto(true);
    }

    if (DebugStepFlags::Get().GetStat2DynInto()) {
        LOG_DEBUGGER(DEBUG) << "JSPtHooks::SingleStep SingleStep from Static";
        debugger_->NotifyPaused({}, OTHER);
        DebugStepFlags::Get().SetStat2DynInto(false);
        return false;
    }

    // pause or step complete
    if (debugger_->NotifySingleStep(location)) {
        // pause for symbol breakpoint
        if (breakOnSymbol_) {
            debugger_->NotifyPaused({}, SYMBOL);
            breakOnSymbol_ = false;
            return true;
        }
        debugger_->NotifyPaused({}, OTHER);
        return true;
    }

    // temporary "safepoint" to handle possible protocol command
    debugger_->NotifyHandleProtocolCommand();

    return false;
}

bool JSPtHooks::NativeOut()
{
    [[maybe_unused]] LocalScope scope(debugger_->vm_);
    if (debugger_->NotifyNativeOut()) {
        debugger_->NotifyPaused({}, NATIVE_OUT);
        return true;
    }

    return false;
}

void JSPtHooks::LoadModule(std::string_view pandaFileName, std::string_view entryPoint)
{
    LOG_DEBUGGER(DEBUG) << "JSPtHooks: LoadModule: " << pandaFileName;

    [[maybe_unused]] LocalScope scope(debugger_->vm_);

    if (debugger_->NotifyScriptParsed(pandaFileName.data(), entryPoint)) {
        if (!debugger_->IsLaunchAccelerateMode()) {
            firstTime_ = true;
        }
    }
}

void JSPtHooks::NativeCalling(const void *nativeAddress)
{
    LOG_DEBUGGER(DEBUG) << "JSPtHooks: NativeCalling, addr = " << nativeAddress;

    [[maybe_unused]] LocalScope scope(debugger_->vm_);

    debugger_->NotifyNativeCalling(nativeAddress);
}

void JSPtHooks::NativeReturn(const void *nativeAddress)
{
    [[maybe_unused]] LocalScope scope(debugger_->vm_);

    debugger_->NotifyNativeReturn(nativeAddress);
}

void JSPtHooks::SendableMethodEntry(JSHandle<Method> method)
{
    LOG_DEBUGGER(DEBUG) << "JSPtHooks: MethodEntry";

    [[maybe_unused]] LocalScope scope(debugger_->vm_);

    if (debugger_->NotifyScriptParsedBySendable(method)) {
        if (!debugger_->IsLaunchAccelerateMode()) {
            firstTime_ = true;
        }
    }
}

void JSPtHooks::DisableFirstTimeFlag()
{
    firstTime_ = false;
}

void JSPtHooks::GenerateAsyncFrames(std::shared_ptr<AsyncStack> asyncStack, bool skipTopFrame)
{
    [[maybe_unused]] LocalScope scope(debugger_->vm_);

    debugger_->GenerateAsyncFrames(asyncStack, skipTopFrame);
}

void JSPtHooks::HitSymbolicBreakpoint()
{
    LOG_DEBUGGER(DEBUG) << "JSPtHooks: HitSymbolicBreakpoint";

    breakOnSymbol_ = true;

    debugger_->SetPauseOnNextByteCode(true);
}

const std::unordered_set<std::string> &JSPtHooks::GetAllRecordNames() const
{
    return debugger_->GetAllRecordNames();
}

void JSPtHooks::SetDebuggerAccessor(const JSHandle<GlobalEnv> &globalEnv)
{
    debugger_->SetDebuggerAccessor(globalEnv);
}
}  // namespace panda::ecmascript::tooling
