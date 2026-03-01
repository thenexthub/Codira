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
    explicit TestXGCEcmaVmAdaptor(napi_env env, [[maybe_unused]] TestXGCEcmaAdaptorValues *value)
        : TestXGCVmAdaptor(env)
    {
    }
};

class TestGCListener : public mem::GCListener {
public:
    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override
    {
        if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
            std::stringstream err;
            err << "Expected GC cause CROSSREF_CAUSE, but got " << task.reason;
            errorMessages_.push_back(err.str());
            return;
        }

        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        if (xrefStorage->Size() != 2U) {
            std::stringstream err;
            err << "Expected xrefs count 2, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }
        xrefStorage->VisitRoots([this, xrefStorage](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            if (!obj->HasInteropIndex()) {
                std::stringstream err;
                err << "Found object " << obj << " in ShredReferenceStorage without interop index";
                errorMessages_.push_back(err.str());
                return;
            }
            auto *xref = xrefStorage->GetReference(obj);
            if (xref->HasETSFlag()) {
                js2Sts_ = xref;
            }
            if (xref->HasJSFlag()) {
                sts2Js_ = xref;
            }
        });
        if (js2Sts_ == nullptr) {
            std::stringstream err;
            err << "No xref JS->STS found";
            errorMessages_.push_back(err.str());
        }
        if (sts2Js_ == nullptr) {
            std::stringstream err;
            err << "No xref STS->JS found";
            errorMessages_.push_back(err.str());
        }
    }

    void GCPhaseStarted(mem::GCPhase phase) override
    {
        auto *stsIface = InteropCtx::Current()->GetSTSVMInterface();
        switch (phase) {
            case mem::GCPhase::GC_PHASE_INITIAL_MARK:
                if (sts2Js_->IsMarked()) {
                    std::stringstream err;
                    err << "Expected STS->JS xref is not marked";
                    errorMessages_.push_back(err.str());
                    return;
                }
                stsIface->MarkFromObject(js2Sts_->GetJsRef());
                if (!sts2Js_->IsMarked()) {
                    std::stringstream err;
                    err << "Expected STS->JS xref is marked";
                    errorMessages_.push_back(err.str());
                    return;
                }
                break;
            default:  // CC-OFF(G.FMT.13-CPP) project code style
                break;
        }
    }

    const std::vector<std::string> &GetErrors() const
    {
        return errorMessages_;
    }

private:
    ets_proxy::SharedReference *js2Sts_ = nullptr;
    ets_proxy::SharedReference *sts2Js_ = nullptr;
    std::vector<std::string> errorMessages_;
};

static TestGCListener g_gcListener;
}  // namespace ark::ets::interop::js

#include "test_module.h"
