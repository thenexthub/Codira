/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "ark_hz/ark_js_runtime.h"
#include <uv.h>
#include "ark_js_runtime.h"
#include "utils/utils.h"

#include "ecmascript/base/string_helper.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "static_core/plugins/ets/runtime/interop_js/timer_helper/interop_timer_helper.h"

#include <iostream>

namespace panda {

std::string GetHelper()
{
    return ecmascript::COMMON_HELP_HEAD_MSG + ecmascript::HELP_OPTION_MSG;
}

ArkJsRuntime::~ArkJsRuntime()
{
    if (engine_ != nullptr) {
        delete engine_;
        engine_ = nullptr;
    }
}

bool ArkJsRuntime::ProcessOptions(int argc, const char **argv, arg_list_t *filenames)
{
    if (argc < 2) {  // 2: at least have two arguments
        std::cerr << GetHelper();
        return false;
    }

    size_t jsvmArgsEndIdx = 0;
    for (int idx = 1; idx < argc; ++idx) {
        if (ecmascript::base::StringHelper::EndsWith(argv[idx], ".abc")) {
            jsvmArgsEndIdx = idx;
            break;
        }
    }

    std::string files = argv[jsvmArgsEndIdx];
    if (files.empty()) {
        std::cerr << "Abc file must pass after arguments to jsvm" << std::endl;
        std::cerr << GetHelper();
        return 1;
    }

    bool retOpt = options_.ParseCommand(jsvmArgsEndIdx, argv);
    if (!retOpt) {
        std::cerr << GetHelper();
        return false;
    }

    *filenames = ecmascript::base::StringHelper::SplitString(files, ":");
    return true;
}

bool ArkJsRuntime::Init()
{
    if (vm_ != nullptr) {
        return true;
    }

    vm_ = JSNApi::CreateEcmaVM(options_);
    if (vm_ == nullptr) {
        std::cerr << "Cannot Create vm" << std::endl;
        return false;
    }

    engine_ = new ArkNativeEngine(vm_, nullptr);

    engine_->SetGetAssetFunc(utils::GetAsset);
    engine_->SetCleanEnv([this] { JSNApi::DestroyJSVM(vm_); });

    return true;
}

bool ArkJsRuntime::Execute(const std::string &filename)
{
    LocalScope scope(vm_);
    return JSNApi::Execute(vm_, filename, options_.GetEntryPoint());
}

uv_loop_t *ArkJsRuntime::GetUVLoop()
{
    return engine_->GetUVLoop();
}

void ArkJsRuntime::Loop()
{
    // 2 here for NativeEngine async_t and 1 for main CallbackPoster
    static constexpr uint32_t MANUALLY_HANDLED_ASYNC_COUNT = 2U + 1U;  // CC-OFF(G.NAM.03-CPP) project code style
    auto *loop = GetUVLoop();
    auto cntHandles = []([[maybe_unused]] uv_handle_t *handle, void *arg) {
        auto *cnt = reinterpret_cast<uint32_t *>(arg);
        (*cnt)++;
    };
    // CC-OFFNXT(G.CTL.03) false positive
    while (true) {
        uint32_t handleCount = 0;
        uv_walk(loop, cntHandles, &handleCount);
        if (handleCount <= MANUALLY_HANDLED_ASYNC_COUNT) {
            break;
        }
        engine_->Loop(LOOP_ONCE);
    }
}

}  // namespace panda
