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

/**
 * @file
 * This file implements the methods of serialize semantic dependency informations.
 */

#include "CompilationCacheSerialization.h"

#include "flatbuffers/CachedASTFormat_generated.h"

#include "Codira/IncrementalCompilation/IncrementalScopeAnalysis.h"

using namespace Codira;
using namespace AST;

// Incremental compilation only enable in codenative backend for now.
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
namespace {
using TUsageOffset = flatbuffers::Offset<CachedASTFormat::Usage>;
using TUseInfoOffset = flatbuffers::Offset<CachedASTFormat::UseInfo>;
using TNameInfoOffset = flatbuffers::Offset<CachedASTFormat::NameInfo>;
using TRelationOffset = flatbuffers::Offset<CachedASTFormat::Relation>;
using TAddedRelationOffset = flatbuffers::Offset<CachedASTFormat::CompilerAddedUsage>;
const int NAME_INFO_CONDITION_SIZE = 2;

std::vector<TNameInfoOffset> CreateNameUsages(
    flatbuffers::FlatBufferBuilder& builder, const std::map<std::string, NameUsage>& nameUsages)
{
    std::vector<TNameInfoOffset> ret(nameUsages.size());
    size_t i = 0;
    for (auto& [name, usage] : std::as_const(nameUsages)) {
        auto nameIdx = builder.CreateSharedString(name);
        auto condIdx =
            builder.CreateVector(std::vector<bool>{usage.hasUnqualifiedUsage, usage.hasUnqualifiedUsageOfImported});
        auto parentIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(usage.parentDecls));
        auto qualifierIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(usage.packageQualifiers));
        ret[i++] = CachedASTFormat::CreateNameInfo(builder, nameIdx, condIdx, parentIdx, qualifierIdx);
    }
    return ret;
}

TUseInfoOffset CreateUseInfos(flatbuffers::FlatBufferBuilder& builder, const UseInfo& usage)
{
    auto usedDeclsIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(usage.usedDecls));
    auto nameUsageIdx = builder.CreateVector(CreateNameUsages(builder, usage.usedNames));
    return CachedASTFormat::CreateUseInfo(builder, usedDeclsIdx, nameUsageIdx);
}

std::vector<TUsageOffset> CreateUsages(flatbuffers::FlatBufferBuilder& builder,
    const std::unordered_map<Ptr<const AST::Decl>, SemaUsage>& unorderedUsageMap)
{
    std::map<std::string, const SemaUsage*> orderedUsages;
    for (auto& [decl, usage] : std::as_const(unorderedUsageMap)) {
        orderedUsages.emplace(decl->rawMangleName, &usage);
    }

    std::vector<TUsageOffset> ret(orderedUsages.size());
    size_t i = 0;
    for (auto& [rawMangleName, usage] : std::as_const(orderedUsages)) {
        auto declIdx = builder.CreateSharedString(rawMangleName);
        auto apiUsageIdx = CreateUseInfos(builder, usage->apiUsages);
        auto bodyUsageIdx = CreateUseInfos(builder, usage->bodyUsages);
        auto boxedTypeIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(usage->boxedTypes));
        auto usageIdx = CachedASTFormat::CreateUsage(builder, declIdx, apiUsageIdx, bodyUsageIdx, boxedTypeIdx);
        ret[i++] = usageIdx;
    }
    return ret;
}

std::vector<TAddedRelationOffset> CreateAddedRelations(flatbuffers::FlatBufferBuilder& builder,
    const std::unordered_map<RawMangledName, std::set<RawMangledName>>& relations)
{
    std::vector<TAddedRelationOffset> ret(relations.size());
    size_t i = 0;
    for (auto& [declMangle, relation] : std::as_const(relations)) {
        auto nameIdx = builder.CreateSharedString(declMangle);
        auto inheritedIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(relation));
        auto relationIdx = CachedASTFormat::CreateCompilerAddedUsage(builder, nameIdx, inheritedIdx);
        ret[i++] = relationIdx;
    }
    return ret;
}

std::vector<TRelationOffset> CreateRelations(
    flatbuffers::FlatBufferBuilder& builder, const std::unordered_map<RawMangledName, SemaRelation>& relations)
{
    std::map<std::string, const SemaRelation*> orderedRelations;
    for (auto& [declMangle, relation] : std::as_const(relations)) {
        orderedRelations.emplace(declMangle, &relation);
    }
    std::vector<TRelationOffset> ret(orderedRelations.size());
    size_t i = 0;
    for (auto& [declMangle, relation] : std::as_const(orderedRelations)) {
        auto nameIdx = builder.CreateSharedString(declMangle);
        auto inheritedIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(relation->inherits));
        auto extendsIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(relation->extends));
        auto extendInterfacesIdx =
            builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(relation->extendedInterfaces));
        auto relationIdx =
            CachedASTFormat::CreateRelation(builder, nameIdx, inheritedIdx, extendsIdx, extendInterfacesIdx);
        ret[i++] = relationIdx;
    }
    return ret;
}
} // namespace

void HashedASTWriter::SetSemanticInfo(const SemanticInfo& info)
{
    semaUsages = CachedASTFormat::CreateSemanticInfo(builder, builder.CreateVector(CreateUsages(builder, info.usages)),
        builder.CreateVector(CreateRelations(builder, info.relations)),
        builder.CreateVector(CreateRelations(builder, info.builtInTypeRelations)),
        builder.CreateVector(CreateAddedRelations(builder, info.compilerAddedUsages)));
}

namespace {
template <typename T> std::set<std::string> GetSetStrings(const T* data)
{
    if (data == nullptr) {
        return {};
    }
    std::set<std::string> strs;
    for (uoffset_t i = 0; i < data->size(); ++i) {
        (void)strs.emplace(data->Get(i)->str());
    }
    return strs;
}

NameUsage GetNameUsage(const CachedASTFormat::NameInfo* info)
{
    if (info == nullptr) {
        return {};
    }
    NameUsage usage;
    usage.parentDecls = GetSetStrings(info->parentDecls());
    usage.packageQualifiers = GetSetStrings(info->qualifiers());
    CODEC_ASSERT(info->conditions()->size() == NAME_INFO_CONDITION_SIZE);
    usage.hasUnqualifiedUsage = info->conditions()->Get(0);
    usage.hasUnqualifiedUsageOfImported = info->conditions()->Get(1);
    return usage;
}

UseInfo GetUseInfo(const CachedASTFormat::UseInfo* usage)
{
    if (usage == nullptr) {
        return {};
    }
    UseInfo apiUsage;
    apiUsage.usedDecls = GetSetStrings(usage->usedDecls());
    for (size_t i = 0; i < usage->usedNames()->size(); ++i) {
        auto nameInfoIdx = usage->usedNames()->Get(static_cast<unsigned>(i));
        apiUsage.usedNames.emplace(nameInfoIdx->name()->str(), GetNameUsage(nameInfoIdx));
    }
    return apiUsage;
}
} // namespace
#endif

SemanticInfo HashedASTLoader::LoadSemanticInfos(
    const CachedASTFormat::HashedPackage& hasedPackage, const RawMangled2DeclMap& mangledName2DeclMap)
{
    SemanticInfo info;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto semaInfo = hasedPackage.semanticInfo();
    if (semaInfo == nullptr) {
        return info;
    }
    for (uoffset_t i = 0; i < semaInfo->usages()->size(); i++) {
        auto usageOff = semaInfo->usages()->Get(i);
        auto found = mangledName2DeclMap.find(usageOff->definition()->str());
        if (found == mangledName2DeclMap.end()) {
            continue;
        }
        auto& usage = info.usages[found->second];
        usage.apiUsages = GetUseInfo(usageOff->apiUsage());
        usage.bodyUsages = GetUseInfo(usageOff->bodyUsage());
        usage.boxedTypes = GetSetStrings(usageOff->boxedTypes());
    }
    for (uoffset_t i = 0; i < semaInfo->relations()->size(); i++) {
        auto relationOff = semaInfo->relations()->Get(i);
        auto defMangle = relationOff->definition()->str();
        auto& relation = info.relations[defMangle];
        relation.inherits = GetSetStrings(relationOff->inherited());
        relation.extends = GetSetStrings(relationOff->extends());
        relation.extendedInterfaces = GetSetStrings(relationOff->extendInterfaces());
    }
    for (uoffset_t i = 0; i < semaInfo->builtInTypeRelations()->size(); i++) {
        auto relationOff = semaInfo->builtInTypeRelations()->Get(i);
        auto builtIntypeName = relationOff->definition()->str();
        auto& relation = info.builtInTypeRelations[builtIntypeName];
        relation.extends = GetSetStrings(relationOff->extends());
        relation.extendedInterfaces = GetSetStrings(relationOff->extendInterfaces());
    }
    for (uoffset_t i = 0; i < semaInfo->compilerAddedUsages()->size(); i++) {
        auto relationOff = semaInfo->compilerAddedUsages()->Get(i);
        auto rawMangle = relationOff->definition()->str();
        info.compilerAddedUsages[rawMangle] = GetSetStrings(relationOff->related());
    }
#endif
    return info;
}
