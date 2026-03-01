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

#include "object_repository.h"

#include "include/tooling/pt_lang_extension.h"
#include "runtime/handle_scope-inl.h"

namespace ark::tooling::inspector {
ObjectRepository::ObjectRepository() : extension_(nullptr), scope_(ManagedThread::GetCurrent())
{
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    extension_ = thread->GetLanguageContext().CreatePtLangExt();
}

RemoteObject ObjectRepository::CreateGlobalObject()
{
    return RemoteObject::Object("[Global]", GLOBAL_OBJECT_ID, "Global object");
}

RemoteObject ObjectRepository::CreateFrameObject(const PtFrame &frame, const std::map<std::string, TypedValue> &locals,
                                                 std::optional<RemoteObject> &objThis)
{
    ASSERT(ManagedThread::GetCurrent()->GetMutatorLock()->HasLock());

    std::vector<PropertyDescriptor> properties;
    properties.reserve(locals.size());
    auto thisParamName = extension_->GetThisParameterName();
    for (const auto &[paramName, value] : locals) {
        auto obj = CreateObject(value);
        if (paramName == thisParamName) {
            objThis.emplace(std::move(obj));
        } else {
            properties.emplace_back(paramName, std::move(obj));
        }
    }

    auto id = counter_++;
    frames_.emplace(id, std::move(properties));

    return RemoteObject::Object("", id, "Frame #" + std::to_string(frame.GetFrameId()));
}

RemoteObject ObjectRepository::CreateObject(TypedValue value)
{
    ASSERT(ManagedThread::GetCurrent()->GetMutatorLock()->HasLock());

    switch (value.GetType()) {
        case panda_file::Type::TypeId::INVALID:
        case panda_file::Type::TypeId::VOID:
            return RemoteObject::Undefined();
        case panda_file::Type::TypeId::U1:
            return RemoteObject::Boolean(value.GetAsU1());
        case panda_file::Type::TypeId::I8:
            return RemoteObject::Number(value.GetAsI8());
        case panda_file::Type::TypeId::U8:
            return RemoteObject::Number(value.GetAsU8());
        case panda_file::Type::TypeId::I16:
            return RemoteObject::Number(value.GetAsI16());
        case panda_file::Type::TypeId::U16:
            return RemoteObject::Number(value.GetAsU16());
        case panda_file::Type::TypeId::I32:
            return RemoteObject::Number(value.GetAsI32());
        case panda_file::Type::TypeId::U32:
            return RemoteObject::Number(value.GetAsU32());
        case panda_file::Type::TypeId::F32:
            return RemoteObject::Number(value.GetAsF32());
        case panda_file::Type::TypeId::F64:
            return RemoteObject::Number(value.GetAsF64());
        case panda_file::Type::TypeId::I64:
            return RemoteObject::Number(value.GetAsI64());
        case panda_file::Type::TypeId::U64:
            return RemoteObject::Number(value.GetAsU64());
        case panda_file::Type::TypeId::REFERENCE:
            return CreateObject(value.GetAsReference());
        case panda_file::Type::TypeId::TAGGED:
            return CreateObject(value.GetAsTagged());
    }
    UNREACHABLE();
}

std::vector<PropertyDescriptor> ObjectRepository::GetProperties(RemoteObjectId id, bool generatePreview)
{
    ASSERT(ManagedThread::GetCurrent()->GetMutatorLock()->HasLock());

    auto properties = GetProperties(id);

    if (generatePreview) {
        for (auto &property : properties) {
            if (property.IsAccessor()) {
                continue;
            }

            RemoteObject &value = property.GetValue();
            auto preview = CreateObjectPreview(value);
            if (preview.has_value()) {
                value.SetObjectPreview(std::move(*preview));
            }
        }
    }

    return properties;
}

std::optional<ObjectPreview> ObjectRepository::CreateObjectPreview(RemoteObject &remobj)
{
    auto valueId = remobj.GetObjectId();
    if (!valueId.has_value()) {
        return {};
    }

    ObjectPreview preview(remobj.GetType(), GetProperties(*valueId));

    return preview;
}

RemoteObject ObjectRepository::CreateObject(coretypes::TaggedValue value)
{
    if (value.IsHeapObject()) {
        return CreateObject(value.GetHeapObject());
    }
    if (value.IsUndefined() || value.IsHole()) {
        return RemoteObject::Undefined();
    }
    if (value.IsNull()) {
        return RemoteObject::Null();
    }
    if (value.IsBoolean()) {
        return RemoteObject::Boolean(value.IsTrue());
    }
    if (value.IsInt()) {
        return RemoteObject::Number(value.GetInt());
    }
    if (value.IsDouble()) {
        return RemoteObject::Number(value.GetDouble());
    }
    UNREACHABLE();
}

RemoteObject ObjectRepository::CreateObject(ObjectHeader *object)
{
    ASSERT(ManagedThread::GetCurrent()->GetMutatorLock()->HasLock());

    if (object == nullptr) {
        return RemoteObject::Null();
    }

    if (auto str = extension_->GetAsString(object)) {
        return RemoteObject::String(*str);
    }

    RemoteObjectId id;

    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto it = std::find_if(objects_.begin(), objects_.end(), [object](auto &p) { return p.second.GetPtr() == object; });
    if (it == objects_.end()) {
        id = counter_++;

        objects_.emplace(std::piecewise_construct, std::forward_as_tuple(id),
                         std::forward_as_tuple(ManagedThread::GetCurrent(), object));
    } else {
        id = it->first;
    }

    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (auto arrayLen = extension_->GetLengthIfArray(object)) {
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        return RemoteObject::Array(extension_->GetClassName(object), *arrayLen, id);
    }

    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return RemoteObject::Object(extension_->GetClassName(object), id);
}

std::vector<PropertyDescriptor> ObjectRepository::GetProperties(RemoteObjectId id)
{
    ASSERT(ManagedThread::GetCurrent()->GetMutatorLock()->HasLock());

    auto fIt = frames_.find(id);
    if (fIt != frames_.end()) {
        ASSERT(objects_.find(id) == objects_.end());
        return fIt->second;
    }

    std::vector<PropertyDescriptor> properties;
    auto propertyHandler = [this, &properties](auto &name, auto value, auto isFinal, auto isAccessor) {
        auto property = isAccessor ? PropertyDescriptor::Accessor(name, CreateObject(value))
                                   : PropertyDescriptor(name, CreateObject(value));
        if (!isAccessor && isFinal) {
            property.SetWritable(false);
        }
        properties.emplace_back(std::move(property));
    };

    if (id == GLOBAL_OBJECT_ID) {
        ASSERT(objects_.find(id) == objects_.end());
        extension_->EnumerateGlobals(propertyHandler);
    } else {
        auto oIt = objects_.find(id);
        if (oIt == objects_.end()) {
            LOG(INFO, DEBUGGER) << "Unknown object ID " << id;
            return {};
        }

        extension_->EnumerateProperties(oIt->second.GetPtr(), propertyHandler);
    }

    return properties;
}
}  // namespace ark::tooling::inspector
