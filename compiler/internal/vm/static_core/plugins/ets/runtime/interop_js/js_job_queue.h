/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_JS_INTEROP_JOB_QUEUE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_JS_INTEROP_JOB_QUEUE_H_

#include "plugins/ets/runtime/job_queue.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::mem {
class Reference;
}  // namespace ark::mem

namespace ark::ets::interop::js {

class JsJobQueue final : public JobQueue {
public:
    /**
     *  This class represents a JS callback associated with the ETS function object: void()
     *  It is used to post callback in the JS job queue and run it in JS env
     */
    class JsCallback {
    public:
        static JsCallback *Create(EtsCoroutine *coro, const EtsObject *callback);

        void Run();

        ~JsCallback();

        NO_COPY_SEMANTIC(JsCallback);
        NO_MOVE_SEMANTIC(JsCallback);

    private:
        explicit JsCallback(mem::Reference *jsCallbackRef) : jsCallbackRef_(jsCallbackRef) {}

        mem::Reference *jsCallbackRef_;

        friend class ark::mem::Allocator;
    };

    void Post(EtsObject *callback) override;
    void CreateLink(EtsObject *source, EtsObject *target) override;

private:
    void CreatePromiseLink(EtsObject *jsObject, EtsPromise *etsPromise);
};

}  // namespace ark::ets::interop::js
#endif  // PANDA_PLUGINS_ETS_RUNTIME_JS_INTEROP_JOB_QUEUE_H_
