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

#ifndef ARK_JS_NAPI_CLI_JS_RUNTIME_H
#define ARK_JS_NAPI_CLI_JS_RUNTIME_H

#include <uv.h>

#include <memory>
#include <string>

#include "libpandabase/utils/pandargs.h"
#include "native_engine/native_engine.h"

namespace panda {

namespace builtins {
class TimerModule;
}  // namespace builtins

class JsRuntime {
public:
    virtual bool ProcessOptions(int argc, const char **argv, arg_list_t *filenames) = 0;

    virtual bool Init() = 0;

    virtual bool Execute(const std::string &fileName) = 0;

    virtual void Loop() = 0;

    virtual uv_loop_t *GetUVLoop() = 0;

    virtual NativeEngine *GetNativeEngine() = 0;

    virtual ~JsRuntime() {}

    static std::unique_ptr<JsRuntime> Create();
};

}  // namespace panda

#endif  // ARK_JS_NAPI_CLI_JS_RUNTIME_H
