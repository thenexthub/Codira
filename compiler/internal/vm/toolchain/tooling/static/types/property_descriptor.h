/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_DESCRIPTOR_H
#define PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_DESCRIPTOR_H

#include "libarkbase/macros.h"

#include "types/remote_object.h"
#include "json_serialization/serializable.h"

#include "remote_object.h"

namespace ark::tooling::inspector {
class PropertyDescriptor final : public JsonSerializable {
public:
    PropertyDescriptor(std::string name, RemoteObject value) : name_(std::move(name)), value_(std::move(value)) {}

    static PropertyDescriptor Accessor(std::string name, RemoteObject getter)
    {
        PropertyDescriptor property(std::move(name), std::move(getter));
        property.isAccessor_ = true;
        return property;
    }

    bool IsAccessor() const
    {
        return isAccessor_;
    }

    bool IsConfigurable() const
    {
        return configurable_;
    }

    bool IsEnumerable() const
    {
        return enumerable_;
    }

    bool IsWritable() const
    {
        ASSERT(!IsAccessor());
        return writable_;
    }

    const RemoteObject &GetGetter() const
    {
        ASSERT(IsAccessor());
        return value_;
    }

    const std::string &GetName() const
    {
        return name_;
    }

    const std::optional<RemoteObject> &GetSymbol() const
    {
        return symbol_;
    }

    const RemoteObject &GetValue() const
    {
        ASSERT(!IsAccessor());
        return value_;
    }

    RemoteObject &GetValue()
    {
        ASSERT(!IsAccessor());
        return value_;
    }

    void SetConfigurable(bool configurable)
    {
        configurable_ = configurable;
    }

    void SetEnumerable(bool enumerable)
    {
        enumerable_ = enumerable;
    }

    void SetSymbol(RemoteObject symbol)
    {
        symbol_.emplace(std::move(symbol));
    }

    void SetWritable(bool writable)
    {
        ASSERT(!IsAccessor());
        writable_ = writable;
    }

    void Serialize(JsonObjectBuilder &builder) const override
    {
        builder.AddProperty("name", name_);

        if (symbol_) {
            builder.AddProperty("symbol", *symbol_);
        }

        if (isAccessor_) {
            builder.AddProperty("get", value_);
        } else {
            builder.AddProperty("value", value_);
            builder.AddProperty("writable", writable_);
        }

        builder.AddProperty("configurable", configurable_);
        builder.AddProperty("enumerable", enumerable_);
    }

private:
    std::string name_;
    std::optional<RemoteObject> symbol_;
    RemoteObject value_;
    bool isAccessor_ {false};
    bool configurable_ {false};
    bool enumerable_ {true};
    bool writable_ {true};
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_DESCRIPTOR_H
