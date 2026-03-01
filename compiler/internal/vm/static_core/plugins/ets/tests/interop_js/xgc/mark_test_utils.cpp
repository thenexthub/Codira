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
#ifndef MARK_TEST_UTILS_H
#define MARK_TEST_UTILS_H

#include <iostream>
#include <array>
#include <thread>
#include <chrono>

#include "hybrid/ecma_vm_interface.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/tests/interop_js/xgc/test_xgc_vm_adaptor.h"

namespace ark::ets::interop::js {

class TestGCListener : public mem::GCListener {
public:
    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override
    {
        this->reason = task.reason;
        if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
            return;
        }
    }

    void GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
        if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
            return;
        }
        this->markNum = 0;
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        xrefStorage->VisitRoots([this, xrefStorage](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            if (obj->HasInteropIndex()) {
                auto *xref = xrefStorage->GetReference(obj);
                if (xref->IsMarked()) {
                    this->markNum++;
                }
            }
        });
    }

    void GetXRefsCount(size_t &jsToStsCount, size_t &stsToJsCount)
    {
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        jsToStsCount = 0;
        stsToJsCount = 0;
        xrefStorage->VisitRoots([xrefStorage, &jsToStsCount, &stsToJsCount](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            if (obj->HasInteropIndex()) {
                auto *xref = xrefStorage->GetReference(obj);
                if (xref->HasETSFlag()) {
                    jsToStsCount++;
                }
                if (xref->HasJSFlag()) {
                    stsToJsCount++;
                }
            }
        });
    }

    int GetMarkNum() const
    {
        return markNum;
    }
    const GCTaskCause &GetReason() const
    {
        return reason;
    }

private:
    int markNum = 0;
    GCTaskCause reason = GCTaskCause::INVALID_CAUSE;
};

class TestModule {
public:
    NO_COPY_SEMANTIC(TestModule);
    NO_MOVE_SEMANTIC(TestModule);
    TestModule() = delete;
    ~TestModule() = default;

    static napi_value Setup(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        ets::PandaEtsVM::GetCurrent()->GetGC()->AddListener(&gcListener);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    }

    static int FetchNumber(napi_env env, napi_value value)
    {
        if (GetValueType(env, value) != napi_number) {
            napi_throw_error(env, nullptr, "The parameter is not number");
        }
        int number;
        napi_get_value_int32(env, value, &number);
        return number;
    }

    static napi_value CheckXRefsNumber(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        const size_t paramNum = 2;
        size_t argc = 0;
        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc != paramNum) {
            napi_throw_error(env, nullptr, "Please transfer the correct parameters");
        }
        std::array<napi_value, paramNum> argv {};
        NAPI_ASSERT_OK(napi_get_cb_info(env, info, &argc, argv.data(), nullptr, nullptr));
        size_t stsXRefNum = static_cast<size_t>(FetchNumber(env, argv[0]));
        size_t jsXRefNum = static_cast<size_t>(FetchNumber(env, argv[1]));
        size_t jsToStsCount;
        size_t stsToJsCount;
        gcListener.GetXRefsCount(jsToStsCount, stsToJsCount);
        if (stsXRefNum != stsToJsCount || jsXRefNum != jsToStsCount) {
            std::stringstream errorMsg;
            if (stsXRefNum != stsToJsCount) {
                errorMsg << "sts->js verification failure,";
                errorMsg << " expected value: " << stsXRefNum << ",";
                errorMsg << " actual value: " << stsToJsCount << "\n";
            }
            if (jsXRefNum != jsToStsCount) {
                errorMsg << "js->sts verification failure,";
                errorMsg << " expected value: " << jsXRefNum << ",";
                errorMsg << " actual value: " << jsToStsCount << "\n";
            }
            napi_throw_error(env, nullptr, errorMsg.str().c_str());
        }
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    }

    static napi_value CheckMarkNumber(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        const size_t paramNum = 1;
        size_t argc = 0;
        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc != paramNum) {
            napi_throw_error(env, nullptr, "Please transfer the correct parameters");
        }
        std::array<napi_value, paramNum> argv {};
        NAPI_ASSERT_OK(napi_get_cb_info(env, info, &argc, argv.data(), nullptr, nullptr));
        int markNum = FetchNumber(env, argv[0]);
        if (markNum != gcListener.GetMarkNum()) {
            std::stringstream errorMsg;
            errorMsg << "markNum verification failure,";
            errorMsg << " expected value: " << markNum << ",";
            errorMsg << " actual value: " << gcListener.GetMarkNum() << "\n";
            napi_throw_error(env, nullptr, errorMsg.str().c_str());
        }
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    }

    static napi_value Init(napi_env env, napi_value exports)
    {
        const std::array desc = {
            napi_property_descriptor {"setup", 0, Setup, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"checkXRefsNumber", 0, CheckXRefsNumber, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"checkMarkNumber", 0, CheckMarkNumber, 0, 0, 0, napi_enumerable, 0},
        };
        napi_define_properties(env, exports, desc.size(), desc.data());
        return exports;
    }

private:
    static TestGCListener gcListener;
};

TestGCListener TestModule::gcListener;

NAPI_MODULE(TEST_MODULE, ark::ets::interop::js::TestModule::Init)

}  // namespace ark::ets::interop::js

#endif  // MARK_TEST_UTILS_H
