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
        if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
            std::stringstream err;
            err << "Expected GC cause CROSSREF_CAUSE, but got " << task.reason;
            errorMessages_.push_back(err.str());
            return;
        }

        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        if (xrefStorage->Size() != EXPECTED_XREFS_COUNT) {
            std::stringstream err;
            err << "Expected 3 xrefs, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }

        size_t idx = 0;
        xrefStorage->VisitRoots([this, xrefStorage, &idx](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            if (!obj->HasInteropIndex()) {
                std::stringstream err;
                err << "Found object " << obj << " in ShredReferenceStorage without interop index";
                errorMessages_.push_back(err.str());
                return;
            }
            auto *xref = xrefStorage->GetReference(obj);
            if (xref->HasETSFlag()) {
                refs_[idx] = xref->GetJsRef();
                ++idx;
            } else {
                std::stringstream err;
                err << "Found unexpected STS->JS xref";
                errorMessages_.push_back(err.str());
            }
        });
        if (idx != EXPECTED_XREFS_COUNT) {
            std::stringstream err;
            err << "Expected 3 JS-STS xrefs but got " << idx;
            errorMessages_.push_back(err.str());
            return;
        }
    }

    void GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        xrefStorage->VisitRoots([this, xrefStorage](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            auto *xref = xrefStorage->GetReference(obj);
            if (!xref->IsMarked()) {
                std::stringstream err;
                err << "Found not marked xref";
                errorMessages_.push_back(err.str());
                return;
            }
        });
    }

    void GCPhaseStarted(mem::GCPhase phase) override
    {
        auto *stsIface = InteropCtx::Current()->GetSTSVMInterface();
        switch (phase) {
            case mem::GCPhase::GC_PHASE_INITIAL_MARK:
                stsIface->MarkFromObject(refs_[0U]);
                break;
            case mem::GCPhase::GC_PHASE_MARK:
                stsIface->MarkFromObject(refs_[1U]);
                break;
            case mem::GCPhase::GC_PHASE_REMARK:
                stsIface->MarkFromObject(refs_[2U]);
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
    // CC-OFFNXT(G.FMT.13-CPP) project code style
    static constexpr size_t EXPECTED_XREFS_COUNT = 3U;
    std::array<napi_ref, EXPECTED_XREFS_COUNT> refs_;
    std::vector<std::string> errorMessages_;
};

static TestGCListener g_gcListener;
}  // namespace ark::ets::interop::js

#include "test_module.h"
