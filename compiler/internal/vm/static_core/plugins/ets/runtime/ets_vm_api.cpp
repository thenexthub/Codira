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

#include <iostream>
#include "ets_vm_api.h"
#include "ets_vm.h"
#include "libarkbase/panda_gen_options/generated/logger_options.h"

#ifdef PANDA_TARGET_OHOS
#include <hilog/log.h>

static void LogPrint([[maybe_unused]] int id, int level, const char *component, [[maybe_unused]] const char *fmt,
                     const char *msg)
{
#ifdef PANDA_OHOS_USE_INNER_HILOG
    constexpr static unsigned int ARK_DOMAIN = 0xD003F00;
    constexpr static auto TAG = "ArkEtsVm";
    constexpr static OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, ARK_DOMAIN, TAG};
    switch (level) {
        case ark::Logger::PandaLog2MobileLog::DEBUG:
            OHOS::HiviewDFX::HiLog::Debug(LABEL, "%{public}s", msg);
            break;
        case ark::Logger::PandaLog2MobileLog::INFO:
            OHOS::HiviewDFX::HiLog::Info(LABEL, "%{public}s", msg);
            break;
        case ark::Logger::PandaLog2MobileLog::ERROR:
            OHOS::HiviewDFX::HiLog::Error(LABEL, "%{public}s", msg);
            break;
        case ark::Logger::PandaLog2MobileLog::FATAL:
            OHOS::HiviewDFX::HiLog::Fatal(LABEL, "%{public}s", msg);
            break;
        case ark::Logger::PandaLog2MobileLog::WARN:
            OHOS::HiviewDFX::HiLog::Warn(LABEL, "%{public}s", msg);
            break;
        default:
            UNREACHABLE();
    }
#else
    switch (level) {
        case ark::Logger::PandaLog2MobileLog::DEBUG:
            OH_LOG_Print(LOG_APP, LOG_DEBUG, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case ark::Logger::PandaLog2MobileLog::INFO:
            OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case ark::Logger::PandaLog2MobileLog::ERROR:
            OH_LOG_Print(LOG_APP, LOG_ERROR, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case ark::Logger::PandaLog2MobileLog::FATAL:
            OH_LOG_Print(LOG_APP, LOG_FATAL, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case ark::Logger::PandaLog2MobileLog::WARN:
            OH_LOG_Print(LOG_APP, LOG_WARN, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        default:
            UNREACHABLE();
    }
#endif  // PANDA_OHOS_USE_INNER_HILOG
}
#else
static void LogPrint([[maybe_unused]] int id, [[maybe_unused]] int level, [[maybe_unused]] const char *component,
                     [[maybe_unused]] const char *fmt, const char *msg)
{
    std::cerr << msg << "\n";
}
#endif  // PANDA_TARGET_OHOS

namespace ark::ets {

bool CreateRuntime(std::function<bool(logger::Options *, RuntimeOptions *)> const &addOptions)
{
    auto runtimeOptions = ark::RuntimeOptions("app");
    runtimeOptions.SetLoadRuntimes({"ets"});
#ifdef PANDA_TARGET_OHOS
    runtimeOptions.SetMobileLog(reinterpret_cast<void *>(LogPrint));
#endif

    ark::logger::Options loggerOptions("app");

    if (!addOptions(&loggerOptions, &runtimeOptions)) {
        return false;
    }

    ark::Logger::Initialize(loggerOptions);

    LOG(DEBUG, RUNTIME) << "CreateRuntime";

#ifdef ARK_HYBRID
    if (!runtimeOptions.WasSetGcType("ets")) {
        runtimeOptions.SetGcType("cmc-gc");
        LOG(INFO, RUNTIME) << "Not set the GC type, and use cmc-gc by default when Ark hybrid mode is enable";
    }
#endif
#ifdef PANDA_JS_ETS_HYBRID_MODE
    if ((runtimeOptions.GetXgcTriggerType() != "never") &&
        (runtimeOptions.GetGcType("ets") != "g1-gc" || runtimeOptions.IsNoAsyncJit())) {
        // XGC is not implemented for other GC types
        LOG(ERROR, RUNTIME) << "GC type must be g1-gc and no-async-jit option must be false";
        return false;
    }
#endif

    if (!ark::Runtime::Create(runtimeOptions)) {
        LOG(ERROR, RUNTIME) << "CreateRuntime: cannot create ets runtime";
        return false;
    }
    return true;
}

bool CreateRuntime(const std::string &stdlibAbc, const std::string &pathAbc, const bool useJit, const bool useAot)
{
    auto addOpts = [&stdlibAbc, &pathAbc, useJit, useAot](logger::Options *loggerOptions,
                                                          ark::RuntimeOptions *runtimeOptions) {
        loggerOptions->SetLogLevel("info");
        runtimeOptions->SetBootPandaFiles({stdlibAbc, pathAbc});
        runtimeOptions->SetPandaFiles({pathAbc});
        runtimeOptions->SetGcTriggerType("heap-trigger");
        runtimeOptions->SetCompilerEnableJit(useJit);
        runtimeOptions->SetEnableAn(useAot);
        runtimeOptions->SetCoroutineImpl("stackful");
        return true;
    };
    return CreateRuntime(addOpts);
}

bool DestroyRuntime()
{
    LOG(DEBUG, RUNTIME) << "DestroyEtsRuntime: enter";
    auto res = ark::Runtime::Destroy();
    LOG(DEBUG, RUNTIME) << "DestroyEtsRuntime: result = " << res;
    return res;
}

std::pair<bool, int> ExecuteModule(std::string_view name)
{
    auto runtime = ark::Runtime::GetCurrent();
    auto pfPath = runtime->GetPandaFiles()[0];
    LOG(INFO, RUNTIME) << "ExecuteEtsModule: '" << pfPath << "'";
    std::string moduleEp = (name.empty() ? "ETSGLOBAL::main" : std::string(name) + ".ETSGLOBAL::main");
    auto res = ark::Runtime::GetCurrent()->ExecutePandaFile(pfPath, moduleEp, {});
    LOG(INFO, RUNTIME) << "ExecuteEtsModule: result = " << (res ? std::to_string(res.Value()) : "failed");
    return res ? std::make_pair(true, res.Value()) : std::make_pair(false, 1);
}

bool BindNative(const char *classDescriptor, const char *methodName, void *impl)
{
    auto *runtime = ark::Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    auto *ext = classLinker->GetExtension(ark::SourceLanguage::ETS);
    auto *klass = ext->GetClass(ark::utf::CStringAsMutf8(classDescriptor));

    if (klass == nullptr) {
        auto *thread = ark::ManagedThread::GetCurrent();
        ASSERT(thread != nullptr);
        thread->ClearException();
        LOG(DEBUG, RUNTIME) << "BindNative: Cannot find class '" << classDescriptor << "'";
        return false;
    }

    auto *method = klass->GetDirectMethod(ark::utf::CStringAsMutf8(methodName));

    if (method == nullptr) {
        ark::ManagedThread::GetCurrent()->ClearException();
        LOG(DEBUG, RUNTIME) << "BindNative: Cannot find method '" << classDescriptor << "." << methodName << "'`";
        return false;
    }

    method->SetCompiledEntryPoint(impl);
    return true;
}

void LogError(const std::string &msg)
{
    LogPrint(0, ark::Logger::PandaLog2MobileLog::ERROR, nullptr, nullptr, msg.c_str());
}

}  // namespace ark::ets
