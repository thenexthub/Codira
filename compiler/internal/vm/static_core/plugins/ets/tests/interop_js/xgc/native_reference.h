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

#ifndef NATIVE_REFERENCE_H
#define NATIVE_REFERENCE_H

#include "native_engine/native_reference.h"
#include "interfaces/inner_api/napi/native_node_hybrid_api.h"

namespace ark::ets::interop::js {

class TestNativeReference : public NativeReference {
public:
    explicit TestNativeReference(void *data) : data_(data) {}

    uint32_t Ref()
    {
        return 0;
    }

    uint32_t Unref()
    {
        return 0;
    }

    napi_value Get()
    {
        return GetNapiValue();
    }

    void *GetData()
    {
        return data_;
    }

    operator napi_value()
    {
        return GetNapiValue();
    }

    void SetDeleteSelf() {}

    uint32_t GetRefCount()
    {
        return 0;
    }

    bool GetFinalRun()
    {
        return false;
    }

    napi_value GetNapiValue()
    {
        return static_cast<napi_value>(0);
    }

private:
    void *data_;
};
}  // namespace ark::ets::interop::js

#endif  // NATIVE_REFERENCE_H