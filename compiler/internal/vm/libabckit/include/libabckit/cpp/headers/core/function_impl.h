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

#ifndef CPP_ABCKIT_CORE_FUNCTION_IMPL_H
#define CPP_ABCKIT_CORE_FUNCTION_IMPL_H

#include "function.h"
#include "function_param.h"
#include "class.h"
#include "module.h"
#include "../config.h"
#include "../graph.h"

namespace abckit::core {

inline Graph Function::CreateGraph() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitGraph *graph = conf->cIapi_->createGraphFromFunction(GetView());
    CheckError(conf);
    return Graph(graph, conf_, GetResource());
}

inline Function Function::SetGraph(const Graph &graph) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cMapi_->functionSetGraph(GetView(), graph.GetResource());
    CheckError(conf);
    return *this;
}

inline const File *Function::GetFile() const
{
    return GetResource();
}

inline std::string Function::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->functionGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Annotation> Function::GetAnnotations() const
{
    std::vector<core::Annotation> anns;

    Payload<std::vector<core::Annotation> *> payload {&anns, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->functionEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *ann, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Annotation> *> *>(data);
            payload.data->push_back(core::Annotation(ann, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return anns;
}

inline bool Function::IsStatic() const
{
    const ApiConfig *conf = GetApiConfig();
    bool result = conf->cIapi_->functionIsStatic(GetView());
    CheckError(conf);
    return result;
}

inline bool Function::IsPublic() const
{
    const ApiConfig *conf = GetApiConfig();
    bool result = conf->cIapi_->functionIsPublic(GetView());
    CheckError(conf);
    return result;
}

inline bool Function::IsProtected() const
{
    const ApiConfig *conf = GetApiConfig();
    bool result = conf->cIapi_->functionIsProtected(GetView());
    CheckError(conf);
    return result;
}

inline bool Function::IsPrivate() const
{
    const ApiConfig *conf = GetApiConfig();
    bool result = conf->cIapi_->functionIsPrivate(GetView());
    CheckError(conf);
    return result;
}

inline bool Function::IsInternal() const
{
    const ApiConfig *conf = GetApiConfig();
    bool result = conf->cIapi_->functionIsInternal(GetView());
    CheckError(conf);
    return result;
}

inline bool Function::IsExternal() const
{
    const ApiConfig *conf = GetApiConfig();
    bool result = conf->cIapi_->functionIsExternal(GetView());
    CheckError(conf);
    return result;
}

inline core::Module Function::GetModule() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreModule *module = conf->cIapi_->functionGetModule(GetView());
    CheckError(conf);
    return core::Module(module, conf_, GetResource());
}

inline core::Class Function::GetParentClass() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreClass *klass = conf->cIapi_->functionGetParentClass(GetView());
    CheckError(conf);
    return core::Class(klass, conf_, GetResource());
}

inline std::vector<core::FunctionParam> Function::GetParameters() const
{
    std::vector<core::FunctionParam> params;
    Payload<std::vector<core::FunctionParam> *> payload {&params, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->functionEnumerateParameters(
        GetView(), &payload, [](AbckitCoreFunctionParam *param, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::FunctionParam> *> *>(data);
            payload.data->push_back(core::FunctionParam(param, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());
    return params;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Function::EnumerateNestedFunctions(const std::function<bool(core::Function)> &cb) const
{
    Payload<const std::function<bool(core::Function)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->functionEnumerateNestedFunctions(
        GetView(), &payload, [](AbckitCoreFunction *nestedFunc, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Function)> &> *>(data);
            return payload.data(core::Function(nestedFunc, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Function::EnumerateNestedClasses(const std::function<bool(core::Class)> &cb) const
{
    Payload<const std::function<bool(core::Class)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->functionEnumerateNestedClasses(
        GetView(), &payload, [](AbckitCoreClass *nestedClass, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Class)> &> *>(data);
            return payload.data(core::Class(nestedClass, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline Function Function::GetParentFunction() const
{
    auto *func = GetApiConfig()->cIapi_->functionGetParentFunction(GetView());
    CheckError(GetApiConfig());
    return Function(func, GetApiConfig(), GetResource());
}

inline core::Namespace Function::GetParentNamespace() const
{
    auto *ns = GetApiConfig()->cIapi_->functionGetParentNamespace(GetView());
    CheckError(GetApiConfig());
    return Namespace(ns, GetApiConfig(), GetResource());
}

inline bool Function::EnumerateAnnotations(const std::function<bool(core::Annotation)> &cb) const
{
    Payload<const std::function<bool(core::Annotation)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->functionEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *anno, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Annotation)> &> *>(data);
            return payload.data(core::Annotation(anno, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Function::IsCtor() const
{
    auto isCtor = GetApiConfig()->cIapi_->functionIsCtor(GetView());
    CheckError(GetApiConfig());
    return isCtor;
}

inline bool Function::IsCctor() const
{
    auto isCtor = GetApiConfig()->cIapi_->functionIsCctor(GetView());
    CheckError(GetApiConfig());
    return isCtor;
}

inline bool Function::IsAnonymous() const
{
    auto isAnonymous = GetApiConfig()->cIapi_->functionIsAnonymous(GetView());
    CheckError(GetApiConfig());
    return isAnonymous;
}

inline Function Function::SetGraph(Graph &graph) const
{
    GetApiConfig()->cMapi_->functionSetGraph(GetView(), graph.GetResource());
    CheckError(GetApiConfig());
    return *this;
}

inline Type Function::GetReturnType() const
{
    auto type = GetApiConfig()->cIapi_->functionGetReturnType(GetView());
    CheckError(GetApiConfig());
    return Type(type, GetApiConfig(), GetResource());
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_FUNCTION_IMPL_H
