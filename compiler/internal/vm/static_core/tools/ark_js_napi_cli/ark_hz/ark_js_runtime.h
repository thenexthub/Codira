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

#ifndef ARK_JS_NAPI_CLI_ARK_HZ_JS_RUNTIME_H
#define ARK_JS_NAPI_CLI_ARK_HZ_JS_RUNTIME_H

#include "js_runtime.h"

#include "ecmascript/js_runtime_options.h"
#include "native_engine/impl/ark/ark_native_engine.h"

#include <memory>

namespace panda {

class ArkJsRuntime : public JsRuntime {
public:
    ~ArkJsRuntime() override;

    bool ProcessOptions(int argc, const char **argv, arg_list_t *filenames) override;

    bool Init() override;

    bool Execute(const std::string &filename) override;

    void Loop() override;

    uv_loop_t *GetUVLoop() override;

    NativeEngine *GetNativeEngine() override
    {
        return engine_;
    }

private:
    ecmascript::JSRuntimeOptions options_;
    ecmascript::EcmaVM *vm_ {nullptr};
    ArkNativeEngine *engine_;
};

}  // namespace panda

#endif  // ARK_JS_NAPI_CLI_ARK_HZ_JS_RUNTIME_H
