/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_FIELD_WRAPPER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_FIELD_WRAPPER_H_

#include <memory>
#include <node_api.h>

#include "plugins/ets/runtime/interop_js/ets_proxy/typed_pointer.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_set.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "runtime/include/field.h"

namespace ark {

namespace ets::interop::js {
class InteropCtx;
class JSRefConvert;
}  // namespace ets::interop::js

class Field;
}  // namespace ark

namespace ark::ets::interop::js::ets_proxy {

class EtsClassWrapper;

class EtsFieldWrapper {
public:
    EtsClassWrapper *GetOwner() const
    {
        return owner_;
    }

    Field *GetField() const
    {
        return field_;
    }

    EtsMethodSet *GetGetterSetterMethod(int argc) const
    {
        return argc == 0 ? getter_ : setter_;
    }

    void SetSetterMethod(EtsMethodSet *setter)
    {
        this->setter_ = setter;
    }

    void SetGetterMethod(EtsMethodSet *getter)
    {
        this->getter_ = getter;
    }

    uint32_t GetObjOffset() const
    {
        return objOffset_;
    }

    template <bool ALLOW_INIT>
    JSRefConvert *GetRefConvert(InteropCtx *ctx);

    napi_property_descriptor MakeInstanceProperty(EtsClassWrapper *owner, Field *field);
    napi_property_descriptor MakeStaticProperty(EtsClassWrapper *owner, Field *field);

    EtsFieldWrapper() = default;
    explicit EtsFieldWrapper(EtsClassWrapper *owner) : owner_(owner) {};
    ~EtsFieldWrapper() = default;
    NO_COPY_SEMANTIC(EtsFieldWrapper);
    NO_MOVE_SEMANTIC(EtsFieldWrapper);

private:
    EtsFieldWrapper(EtsClassWrapper *owner, Field *field) : owner_(owner), field_(field), lazyRefconvertLink_(field)
    {
        static_assert(std::is_trivially_destructible_v<EtsFieldWrapper>);

        if (field->IsStatic()) {
            objOffset_ = field_->GetOffset() + EtsClass::GetRuntimeClassOffset();
        } else {
            objOffset_ = field_->GetOffset();
        }
    }

    EtsClassWrapper *owner_ {};
    Field *field_ {};
    EtsMethodSet *getter_ {};
    EtsMethodSet *setter_ {};
    TypedPointer<const Field, JSRefConvert> lazyRefconvertLink_ {};

    uint32_t objOffset_ {};
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_FIELD_WRAPPER_H_
