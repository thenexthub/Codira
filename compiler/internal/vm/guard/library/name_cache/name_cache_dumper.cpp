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

#include "name_cache_dumper.h"

#include "libarkbase/utils/json_builder.h"

#include "logger.h"
#include "obfuscator/member_linker.h"
#include "obfuscator/obfuscator_data_manager.h"
#include "util/file_util.h"

namespace {
bool HasContent(const std::shared_ptr<ark::guard::ObjectNameCache> &cache)
{
    if (!cache->obfName.empty()) {
        return true;
    }

    for (const auto &field : cache->fields) {
        if (!field.first.empty() && !field.second.empty()) {
            return true;
        }
    }

    for (const auto &method : cache->methods) {
        if (!method.first.empty() && !method.second.empty()) {
            return true;
        }
    }

    for (const auto &object : cache->objects) {
        if (!object.first.empty() && HasContent(object.second)) {
            return true;
        }
    }
    return false;
}

std::function<void(ark::JsonObjectBuilder &)> ObjectNameCacheToJson(
    const std::shared_ptr<ark::guard::ObjectNameCache> &objects)
{
    return [objects](ark::JsonObjectBuilder &builder) {
        if (!objects->obfName.empty()) {
            builder.AddProperty(ark::guard::NameCacheConstants::OBF_NAME, objects->obfName);
        }
        for (const auto &field : objects->fields) {
            if (!field.first.empty() && !field.second.empty()) {
                builder.AddProperty(field.first, field.second);
            }
        }
        for (const auto &method : objects->methods) {
            if (!method.first.empty() && !method.second.empty()) {
                builder.AddProperty(method.first, method.second);
            }
        }
        for (const auto &object : objects->objects) {
            if (!object.first.empty() && HasContent(object.second)) {
                builder.AddProperty(object.first, ObjectNameCacheToJson(object.second));
            }
        }
    };
}

std::shared_ptr<ark::guard::ObjectNameCache> GetParentObjectNameCache(ark::guard::ObfuscatorDataManager &manager)
{
    auto parentData = manager.GetParentData();
    if (!parentData) {
        return nullptr;
    }
    return parentData->GetObjectNameCache();
}
}  // namespace

ark::guard::ErrorCode ark::guard::NameCacheDumper::Dump()
{
    if (defaultNameCache_.empty() && printNameCache_.empty()) {
        LOG_E << "Configuration parsing failed: the value of field defaultNameCachePath and printNameCache in the "
                 "configuration file is invalid";
        return ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR;
    }

    this->fileView_.ModulesAccept(*this);

    auto json = BuildJson();
    if (json.empty()) {
        LOG_E << "Failed to generate nameCache json";
        return ErrorCode::GENERIC_ERROR;
    }

    if (!defaultNameCache_.empty()) {
        FileUtil::WriteFile(defaultNameCache_, json);
    }
    if (!printNameCache_.empty()) {
        FileUtil::WriteFile(printNameCache_, json);
    }

    return ErrorCode::ERR_SUCCESS;
}

bool ark::guard::NameCacheDumper::Visit(abckit_wrapper::Module *module)
{
    if (module->IsExternal()) {
        return true;
    }

    ObfuscatorDataManager manager(module);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get module obfuscate data failed, module:" << module->GetFullyQualifiedName();
        return false;
    }

    auto cache = std::make_shared<ObjectNameCache>();
    cache->obfName = obfuscateData->GetObfName();
    obfuscateData->SetObjectNameCache(cache);

    module->ChildrenAccept(*this);

    moduleCaches_.emplace(obfuscateData->GetName(), std::move(cache));
    return true;
}

bool ark::guard::NameCacheDumper::VisitNamespace(abckit_wrapper::Namespace *ns)
{
    ObfuscatorDataManager manager(ns);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get namespace obfuscate data failed, namespace:" << ns->GetFullyQualifiedName();
        return false;
    }
    auto parentObjectNameCache = GetParentObjectNameCache(manager);
    if (!parentObjectNameCache) {
        LOG_E << "Get namespace parent obfuscate data failed, namespace:" << ns->GetFullyQualifiedName();
        return false;
    }

    auto cache = std::make_shared<ObjectNameCache>();
    cache->obfName = obfuscateData->GetObfName();
    obfuscateData->SetObjectNameCache(cache);

    ns->ChildrenAccept(*this);

    parentObjectNameCache->objects.emplace(obfuscateData->GetName(), std::move(cache));
    return true;
}

bool ark::guard::NameCacheDumper::VisitMethod(abckit_wrapper::Method *method)
{
    if (method->IsConstructor()) {
        return true;
    }
    auto lastMember = MemberLinker::LastMember(method);
    ObfuscatorDataManager curManager(method);
    ObfuscatorDataManager lastManager(lastMember);
    auto curData = curManager.GetData();
    auto lastData = lastManager.GetData();
    if (curData == nullptr || lastData == nullptr) {
        LOG_E << "Get method obfuscate data failed, method:" << method->GetFullyQualifiedName();
        return false;
    }
    auto parentObjectNameCache = GetParentObjectNameCache(curManager);
    if (!parentObjectNameCache) {
        LOG_E << "Get method parent obfuscate data failed, method:" << method->GetFullyQualifiedName();
        return false;
    }

    parentObjectNameCache->methods.emplace(curData->GetName(), lastData->GetObfName());
    return true;
}

bool ark::guard::NameCacheDumper::VisitField(abckit_wrapper::Field *field)
{
    auto lastMember = MemberLinker::LastMember(field);
    ObfuscatorDataManager curManager(field);
    ObfuscatorDataManager lastManager(lastMember);
    auto curData = curManager.GetData();
    auto lastData = lastManager.GetData();
    if (curData == nullptr || lastData == nullptr) {
        LOG_E << "Get field obfuscate data failed, field:" << field->GetFullyQualifiedName();
        return false;
    }
    auto parentObjectNameCache = GetParentObjectNameCache(curManager);
    if (!parentObjectNameCache) {
        LOG_E << "Get field parent obfuscate data failed, field:" << field->GetFullyQualifiedName();
        return false;
    }

    parentObjectNameCache->fields.emplace(curData->GetName(), lastData->GetObfName());
    return true;
}

bool ark::guard::NameCacheDumper::VisitClass(abckit_wrapper::Class *clazz)
{
    if (clazz->IsExternal()) {
        return true;
    }

    ObfuscatorDataManager manager(clazz);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get class obfuscate data failed, class:" << clazz->GetFullyQualifiedName();
        return false;
    }
    auto parentObjectNameCache = GetParentObjectNameCache(manager);
    if (!parentObjectNameCache) {
        LOG_E << "Get class parent obfuscate data failed, class:" << clazz->GetFullyQualifiedName();
        return false;
    }

    auto cache = std::make_shared<ObjectNameCache>();
    cache->obfName = obfuscateData->GetObfName();
    obfuscateData->SetObjectNameCache(cache);

    clazz->ChildrenAccept(*this);

    parentObjectNameCache->objects.emplace(obfuscateData->GetName(), std::move(cache));

    return true;
}

bool ark::guard::NameCacheDumper::VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai)
{
    ObfuscatorDataManager manager(ai);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get annotation interface obfuscate data failed, annotation interface:" << ai->GetFullyQualifiedName();
        return false;
    }
    auto parentObjectNameCache = GetParentObjectNameCache(manager);
    if (!parentObjectNameCache) {
        LOG_E << "Get annotation interface parent obfuscate data failed, annotation interface:"
              << ai->GetFullyQualifiedName();
        return false;
    }

    auto cache = std::make_shared<ObjectNameCache>();
    cache->obfName = obfuscateData->GetObfName();
    obfuscateData->SetObjectNameCache(cache);

    parentObjectNameCache->objects.emplace(obfuscateData->GetName(), std::move(cache));

    return true;
}

std::string ark::guard::NameCacheDumper::BuildJson()
{
    if (moduleCaches_.empty()) {
        LOG_E << "NameCaches is empty, failed to build nameCache json";
        return "";
    }

    JsonObjectBuilder builder;
    for (const auto &[name, cache] : moduleCaches_) {
        builder.AddProperty(name, ObjectNameCacheToJson(cache));
    }
    builder.AddProperty(ObfuscationToolConstants::NAME, ObfuscationToolConstants::VERSION);
    return std::move(builder).Build();
}