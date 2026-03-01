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

class TestXGCEcmaAdaptorValues {
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
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        if (xrefStorage->Size() != 1U) {
            std::stringstream err;
            err << "Expected 1 xref, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }

        ObjectHeader *proxy = nullptr;
        xrefStorage->VisitRoots([this, xrefStorage, &proxy](const mem::GCRoot &root) {
            proxy = root.GetObjectHeader();
            xref_ = xrefStorage->GetReference(EtsObject::FromCoreType(proxy));
        });
        if (proxy == nullptr) {
            std::stringstream err;
            err << "STS proxy object not found";
            errorMessages_.push_back(err.str());
            return;
        }
        auto fields = proxy->ClassAddr<Class>()->GetInstanceFields();
        if (fields.size() != 1) {
            std::stringstream err;
            err << "Expected 1 field, but got " << fields.size();
            errorMessages_.push_back(err.str());
            return;
        }
        if (fields[0].GetType().GetId() != panda_file::Type::TypeId::REFERENCE) {
            std::stringstream err;
            err << "Expected the reference field, but got " << fields[0].GetType();
            errorMessages_.push_back(err.str());
            return;
        }
        child_ = ObjectAccessor::GetFieldObject(proxy, fields[0]);
        if (child_ == nullptr) {
            std::stringstream err;
            err << "Expected 1 field, but got " << fields.size();
            errorMessages_.push_back(err.str());
            return;
        }
    }

    void GCPhaseStarted(mem::GCPhase phase) override
    {
        auto *stsIface = InteropCtx::Current()->GetSTSVMInterface();
        if (phase != mem::GCPhase::GC_PHASE_MARK) {
            return;
        }
        auto *gc = ets::PandaEtsVM::GetCurrent()->GetGC();
        if (gc->IsMarked(child_)) {
            std::stringstream err;
            err << "Expected child object is not marked";
            errorMessages_.push_back(err.str());
            return;
        }
        stsIface->MarkFromObject(xref_->GetJsRef());
        if (!gc->IsMarked(child_)) {
            std::stringstream err;
            err << "Expected child object is marked";
            errorMessages_.push_back(err.str());
            return;
        }
    }

    const std::vector<std::string> &GetErrors() const
    {
        return errorMessages_;
    }

private:
    ets_proxy::SharedReference *xref_ = nullptr;
    ObjectHeader *child_ = nullptr;
    std::vector<std::string> errorMessages_;
};

static TestGCListener g_gcListener;
}  // namespace ark::ets::interop::js

#include "test_module.h"
