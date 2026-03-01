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
#include <array>

#include "hybrid/ecma_vm_interface.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/tests/interop_js/xgc/test_xgc_vm_adaptor.h"

namespace ark::ets::interop::js {

struct TestXGCEcmaAdaptorValues {
    std::vector<std::string> GetErrors()
    {
        return {};
    }
};
static TestXGCEcmaAdaptorValues g_xgcAdaptorValues;

class TestXGCEcmaVmAdaptor : public TestXGCVmAdaptor {
public:
    TestXGCEcmaVmAdaptor(napi_env env, [[maybe_unused]] TestXGCEcmaAdaptorValues *value) : TestXGCVmAdaptor(env) {}
};

class TestGCListener : public mem::GCListener {
public:
    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override
    {
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        if (xrefStorage->Size() != 0U) {
            std::stringstream err;
            err << "Expected 0 refs before XGC, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }
    }

    void GCPhaseStarted(mem::GCPhase phase) override
    {
        if (phase != mem::GCPhase::GC_PHASE_MARK) {
            return;
        }
        auto *coro = EtsCoroutine::GetCurrent();
        auto *ctx = InteropCtx::Current(coro);
        napi_env env = ctx->GetJSEnv();
        napi_value global;
        NAPI_CHECK_FATAL(napi_get_global(env, &global));
        napi_value createRefsFunction;
        NAPI_CHECK_FATAL(napi_get_named_property(env, global, "createRefs", &createRefsFunction));
        napi_value etsVm;
        NAPI_CHECK_FATAL(napi_get_named_property(env, global, "etsVm", &etsVm));
        {
            // Create 2 shared references on Concurrent phase, should be processed on Remark phase
            os::memory::ReadLockHolder lock(*coro->GetPandaVM()->GetRendezvous()->GetMutatorLock());
            ScopedNativeCodeThread nativeCode(coro);
            NAPI_CHECK_FATAL(napi_call_function(env, global, createRefsFunction, 1U, &etsVm, nullptr));
        }
        auto *xrefStorage = ctx->GetSharedRefStorage();
        if (xrefStorage->Size() != 2U) {
            std::stringstream err;
            err << "Expected 2 refs after createRefs call on Concurrent, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }
        xrefStorage->VisitRoots([this, xrefStorage](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            auto *xref = xrefStorage->GetReference(obj);
            if (xref->IsMarked()) {
                std::stringstream err;
                err << "Found marked xref before Concurrent mark: " << xref;
                errorMessages_.push_back(err.str());
            }
        });
    }

    void GCPhaseFinished(mem::GCPhase phase) override
    {
        if (phase != mem::GCPhase::GC_PHASE_REMARK) {
            return;
        }
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        if (xrefStorage->Size() != 2U) {
            std::stringstream err;
            err << "Expected 2 refs after Remark, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }
        xrefStorage->VisitRoots([this, xrefStorage](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            auto *xref = xrefStorage->GetReference(obj);
            if (!xref->IsMarked()) {
                std::stringstream err;
                err << "Found not marked xref after Remark: " << xref;
                errorMessages_.push_back(err.str());
            }
        });
    }

    void GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        if (xrefStorage->Size() != 2U) {
            std::stringstream err;
            err << "Expected 2 refs after XGC, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }
    }

    const std::vector<std::string> &GetErrors() const
    {
        return errorMessages_;
    }

private:
    std::vector<std::string> errorMessages_;
};

static TestGCListener g_gcListener;
}  // namespace ark::ets::interop::js

#include "test_module.h"
