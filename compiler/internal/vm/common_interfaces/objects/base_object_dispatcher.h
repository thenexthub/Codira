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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H

#include "objects/base_object.h"
#include "objects/base_object_accessor.h"
#include "objects/base_object_descriptor.h"
#include "objects/base_type.h"
#include "objects/base_type_converter.h"
#include "objects/static_object_accessor_interface.h"
#include "objects/static_type_converter_interface.h"
#include "thread/thread_holder.h"

namespace common {
class BaseObjectDispatcher {
enum class ObjectType : uint8_t { STATIC = 0x0, DYNAMIC, UNKNOWN };
public:
    // Singleton
    static BaseObjectDispatcher& GetDispatcher()
    {
        static BaseObjectDispatcher instance;
        return instance;
    }

    void RegisterDynamicTypeConverter(DynamicTypeConverterInterface *dynTypeConverter)
    {
        dynTypeConverter_ = dynTypeConverter;
    }

    void RegisterDynamicObjectAccessor(DynamicObjectAccessorInterface *dynObjAccessor)
    {
        dynObjAccessor_ = dynObjAccessor;
    }

    void RegisterDynamicObjectDescriptor(DynamicObjectDescriptorInterface *dynObjDescriptor)
    {
        dynObjDescriptor_ = dynObjDescriptor;
    }

    void RegisterStaticTypeConverter(StaticTypeConverterInterface *stcTypeConverter)
    {
        stcTypeConverter_ = stcTypeConverter;
    }

    void RegisterStaticObjectAccessor(StaticObjectAccessorInterface *stcObjAccessor)
    {
        stcObjAccessor_ = stcObjAccessor;
    }

    void RegisterStaticObjectDescriptor(StaticObjectDescriptorInterface *stcObjDescriptor)
    {
        stcObjDescriptor_ = stcObjDescriptor;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    JSTaggedValue GetTaggedProperty(ThreadHolder *thread, const BaseObject *obj, const char *name) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->GetProperty(thread, obj, name);
        } else if constexpr (objType == ObjectType::STATIC) {
            BoxedValue value = stcObjAccessor_->GetProperty(thread, obj, name);
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnwrapBoxed(value));
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->GetProperty(thread, obj, name);
            } else {  // NOLINT(readability-else-after-return)
                BoxedValue value = stcObjAccessor_->GetProperty(thread, obj, name);
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnwrapBoxed(value));
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    BoxedValue GetBoxedProperty(ThreadHolder *thread, const BaseObject *obj, const char *name) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            JSTaggedValue value = dynObjAccessor_->GetProperty(thread, obj, name);
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->GetProperty(thread, obj, name);
        } else {
            if (obj->IsDynamic()) {
                JSTaggedValue value = dynObjAccessor_->GetProperty(thread, obj, name);
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->GetProperty(thread, obj, name);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    JSTaggedValue GetTaggedElementByIdx(ThreadHolder *thread, const BaseObject *obj, const uint32_t index) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->GetElementByIdx(thread, obj, index);
        } else if constexpr (objType == ObjectType::STATIC) {
            BoxedValue value = stcObjAccessor_->GetElementByIdx(thread, obj, index);
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnwrapBoxed(value));
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->GetElementByIdx(thread, obj, index);
            } else {  // NOLINT(readability-else-after-return)
                BoxedValue value = stcObjAccessor_->GetElementByIdx(thread, obj, index);
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnwrapBoxed(value));
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    BoxedValue GetBoxedElementByIdx(ThreadHolder *thread, const BaseObject *obj, const uint32_t index) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            JSTaggedValue value = dynObjAccessor_->GetElementByIdx(thread, obj, index);
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->GetElementByIdx(thread, obj, index);
        } else {
            if (obj->IsDynamic()) {
                JSTaggedValue value = dynObjAccessor_->GetElementByIdx(thread, obj, index);
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->GetElementByIdx(thread, obj, index);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetTaggedProperty(ThreadHolder *thread, BaseObject *obj, const char *name, JSTaggedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->SetProperty(thread, obj, name, value);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->SetProperty(thread, obj, name,
                                                stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->SetProperty(thread, obj, name, value);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return stcObjAccessor_->SetProperty(
                    thread, obj, name, stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetBoxedProperty(ThreadHolder *thread, BaseObject *obj, const char *name, BoxedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            auto wrapedValue = dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnwrapBoxed(value));
            return dynObjAccessor_->SetProperty(thread, obj, name, wrapedValue);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->SetProperty(thread, obj, name, value);
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                auto wrapedValue = dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnwrapBoxed(value));
                return dynObjAccessor_->SetProperty(thread, obj, name, wrapedValue);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->SetProperty(thread, obj, name, value);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetTaggedElementByIdx(ThreadHolder *thread, BaseObject *obj, const uint32_t index, JSTaggedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->SetElementByIdx(thread, obj, index, value);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return stcObjAccessor_->SetElementByIdx(
                thread, obj, index, stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->SetElementByIdx(thread, obj, index, value);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return stcObjAccessor_->SetElementByIdx(
                    thread, obj, index, stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetBoxedElementByIdx(ThreadHolder *thread, BaseObject *obj, const uint32_t index, BoxedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return dynObjAccessor_->SetElementByIdx(
                thread, obj, index, dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnwrapBoxed(value)));
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->SetElementByIdx(thread, obj, index, value);
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return dynObjAccessor_->SetElementByIdx(
                    thread, obj, index, dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnwrapBoxed(value)));
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->SetElementByIdx(thread, obj, index, value);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool HasProperty(ThreadHolder *thread, const BaseObject *obj, const char *name) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->HasProperty(thread, obj, name);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->HasProperty(thread, obj, name);
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->HasProperty(thread, obj, name);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->HasProperty(thread, obj, name);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool HasElementByIdx(ThreadHolder *thread, const BaseObject *obj, const uint32_t index) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->HasElementByIdx(thread, obj, index);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->HasElementByIdx(thread, obj, index);
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->HasElementByIdx(thread, obj, index);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->HasElementByIdx(thread, obj, index);
            }
        }
    }

private:
    DynamicTypeConverterInterface *dynTypeConverter_;
    StaticTypeConverterInterface *stcTypeConverter_;
    DynamicObjectAccessorInterface *dynObjAccessor_;
    StaticObjectAccessorInterface *stcObjAccessor_;
    DynamicObjectDescriptorInterface *dynObjDescriptor_;
    StaticObjectDescriptorInterface *stcObjDescriptor_;
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H