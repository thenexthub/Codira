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
 * This file implements the serialization of cached data for incremental compilation.
 */

#include "CompilationCacheSerialization.h"

#include "flatbuffers/CachedASTFormat_generated.h"

#include "Codira/AST/Walker.h"
#include "Codira/Basic/Version.h"
#include "Codira/Parse/ASTHasher.h"
#include "Codira/Utils/CheckUtils.h"
#include "Codira/AST/ASTCasting.h"

using namespace Codira;
using namespace AST;
using namespace ::flatbuffers;
using namespace CachedASTFormat;

namespace {
Offset<MemberDecl> Write(FlatBufferBuilder& builder, MemberDeclCache&& decl)
{
    auto man = builder.CreateSharedString(decl.rawMangle);
    auto fil = builder.CreateSharedString(decl.gvid.file);
    auto gvid = CreateGlobalVarIndex(builder, fil, decl.gvid.id);
    Offset<Vector<Offset<MemberDecl>>> memberOffset{0};
    if (!decl.members.empty()) {
        size_t i{0};
        std::vector<Offset<MemberDecl>> members(decl.members.size());
        for (std::move_iterator it{decl.members.begin()}; it != std::move_iterator{decl.members.end()}; ++it) {
            members[i++] = Write(builder, *it);
        }
        memberOffset = builder.CreateVector(members);
    }
    auto cgMangle = builder.CreateSharedString(decl.cgMangle);
    return CreateMemberDecl(
        builder, man, decl.sigHash, decl.srcUse, decl.bodyHash, decl.astKind, decl.isGV, gvid, memberOffset, cgMangle);
}

Offset<TopDecl> Write(FlatBufferBuilder& builder, const RawMangledName& mangle, TopLevelDeclCache&& decl)
{
    auto man = builder.CreateSharedString(mangle);
    auto fil = builder.CreateSharedString(decl.gvid.file);
    auto gvid = CreateGlobalVarIndex(builder, fil, decl.gvid.id);
    // write members in the order of increasing gvid
    // static variables first, other member decls following, each group in source code order
    Offset<Vector<Offset<MemberDecl>>> memberOffset{0};
    if (!decl.members.empty()) {
        std::vector<Offset<MemberDecl>> members(decl.members.size());
        size_t i{0};
        for (std::move_iterator it{decl.members.begin()}; it != std::move_iterator{decl.members.end()}; ++it) {
            if (it->isGV) {
                members[i++] = Write(builder, *it);
            }
        }
        for (std::move_iterator it{decl.members.begin()}; it != std::move_iterator{decl.members.end()}; ++it) {
            if (!it->isGV) {
                members[i++] = Write(builder, *it);
            }
        }
        memberOffset = builder.CreateVector(members);
    }
    Offset<Vector<Offset<String>>> extendsOffset{0};
    if (!decl.extends.empty()) {
        std::vector<Offset<String>> members(decl.extends.size());
        size_t i{0};
        for (std::move_iterator it{decl.extends.begin()}; it != std::move_iterator{decl.extends.end()}; ++it) {
            members[i++] = builder.CreateSharedString(*it);
        }
        extendsOffset = builder.CreateVector(members);
    }
    auto cgMangle = builder.CreateSharedString(decl.cgMangle);
    return CreateTopDecl(builder, man, decl.sigHash, decl.srcUse, decl.bodyHash, decl.astKind, decl.isGV,
        decl.instVarHash, decl.virtHash, gvid, memberOffset, extendsOffset, cgMangle);
}

Offset<Vector<Offset<TopDecl>>> WriteCachedAST(FlatBufferBuilder& builder, ASTCache&& ast,
    std::vector<const Decl*>&& order)
{
    if (ast.empty()) {
        return 0;
    }
    std::vector<Offset<TopDecl>> res{};
    for (auto decl : order) {
        CODEC_ASSERT(decl != nullptr);
        // rawMangleName of main is moved into its desugared decl. Get that mangle in this case
        auto& mangled{(Is<MainDecl>(decl) ?
            *StaticCast<MainDecl>(*decl).desugarDecl : *decl).rawMangleName};
        res.push_back(Write(builder, mangled, std::move(ast.at(mangled))));
    }
    return builder.CreateVector(res);
}

Offset<Vector<Offset<TopDecl>>> WriteImported(FlatBufferBuilder& builder, ASTCache&& imports)
{
    if (imports.empty()) {
        return 0;
    }
    std::vector<Offset<TopDecl>> res(imports.size());
    size_t i{0};
    for (std::move_iterator it{imports.begin()}; it != std::move_iterator{imports.end()}; ++it) {
        res[i++] = Write(builder, it->first, std::move(it->second));
    }
    return builder.CreateVector(res);
}

template <typename T>
std::optional<std::pair<Ptr<const AST::Decl>, std::vector<Ptr<const AST::Decl>>>> GetDepRelationAsDecl(
    const RawMangledName& declMangel, const T& dependencies, const RawMangled2DeclMap& mangledName2DeclMap)
{
    auto it = mangledName2DeclMap.find(declMangel);
    if (it == mangledName2DeclMap.end()) {
        return {};
    }
    Ptr<const AST::Decl> decl = it->second.get();
    std::vector<Ptr<const AST::Decl>> deps;
    for (auto& d : std::as_const(dependencies)) {
        it = mangledName2DeclMap.find(d);
        if (it != mangledName2DeclMap.end()) {
            deps.emplace_back(it->second.get());
        }
    }
    return std::make_pair(decl, deps);
}

void LoadCHIROptInfo(const CachedASTFormat::HashedPackage& package, const RawMangled2DeclMap& mangledName2DeclMap,
    CompilationCache& cached)
{
    if (package.varAndFunc()) {
        for (uoffset_t i = 0; i < package.varAndFunc()->size(); i++) {
            auto declDep = package.varAndFunc()->Get(i);
            std::string decl = declDep->decl()->str();
            std::vector<std::string> dependency;
            if (!declDep->dependency()) {
                continue;
            }
            for (uoffset_t j = 0; j < declDep->dependency()->size(); j++) {
                (void)dependency.emplace_back(declDep->dependency()->Get(j)->str());
            }
            if (auto relation = GetDepRelationAsDecl(decl, dependency, mangledName2DeclMap)) {
                (void)cached.varAndFuncDep.emplace_back(relation.value());
            }
        }
    }

    if (package.chirOptInfo()) {
        for (uoffset_t i = 0; i < package.chirOptInfo()->size(); i++) {
            auto effectMap = package.chirOptInfo()->Get(i);
            if (!effectMap->effectedDecls()) {
                continue;
            }
            auto& effectItem = cached.chirOptInfo[effectMap->srcDecl()->str()];
            for (uoffset_t j = 0; j < effectMap->effectedDecls()->size(); j++) {
                effectItem.emplace(effectMap->effectedDecls()->Get(j)->str());
            }
        }
    }
}

void LoadVirtualFuncDep(const CachedASTFormat::HashedPackage& package, CompilationCache& cached)
{
    if (package.virtualDep()) {
        for (uoffset_t i = 0; i < package.virtualDep()->size(); i++) {
            auto dep = package.virtualDep()->Get(i);
            cached.virtualFuncDep[dep->raw()->str()] = dep->wrapper()->str();
        }
    }
}

void LoadVarInitDep(const CachedASTFormat::HashedPackage& package, CompilationCache& cached)
{
    if (package.varInitDep()) {
        for (uoffset_t i = 0; i < package.varInitDep()->size(); i++) {
            auto dep = package.varInitDep()->Get(i);
            cached.varInitDepMap[dep->raw()->str()] = dep->wrapper()->str();
        }
    }
}

void LoadCCOutFunc(const CachedASTFormat::HashedPackage& package, CompilationCache& cached)
{
    if (package.ccOutFuncs()) {
        for (uoffset_t i = 0; i < package.ccOutFuncs()->size(); i++) {
            cached.ccOutFuncs.emplace(package.ccOutFuncs()->Get(i)->str());
        }
    }
}
} // namespace

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
namespace Codira {
bool WriteCache(const Package& pkg, CompilationCache&& cachedInfo, std::vector<const AST::Decl*>&& order,
    const std::string& path)
{
    HashedASTWriter writer;
    writer.SetImportSpecs(pkg);
    writer.SetLambdaCounter(cachedInfo.lambdaCounter);
    writer.SetEnvClassCounter(cachedInfo.envClassCounter);
    writer.SetStringLiteralCounter(cachedInfo.stringLiteralCounter);
    writer.SetCompileArgs(cachedInfo.compileArgs);
    writer.SetVarAndFuncDependency(cachedInfo.varAndFuncDep);
    writer.SetCHIROptInfo(cachedInfo.chirOptInfo);
    writer.SetVirtualFuncDep(cachedInfo.virtualFuncDep);
    writer.SetVarInitDep(cachedInfo.varInitDepMap);
    writer.SetCCOutFuncs(cachedInfo.ccOutFuncs);
    writer.SetSemanticInfo(cachedInfo.semaInfo);
    writer.SetBitcodeFilesName(cachedInfo.bitcodeFilesName);
    writer.WriteAllDecls(
        std::move(cachedInfo.curPkgASTCache), std::move(cachedInfo.importedASTCache), std::move(order));

    return FileUtil::WriteBufferToASTFile(path, writer.AST2FB(pkg.fullPackageName));
}
}
#endif

void HashedASTWriter::WriteAllDecls(ASTCache&& ast, ASTCache&& imports, std::vector<const Decl*>&& order)
{
    allAST = WriteCachedAST(builder, std::move(ast), std::move(order));
    importedDecls = WriteImported(builder, std::move(imports));
}

void HashedASTWriter::SetImportSpecs(const Package& package)
{
    specs = ASTHasher::HashSpecs(package);
}

void HashedASTWriter::SetLambdaCounter(uint64_t counter)
{
    lambdaCounter = counter;
}

void HashedASTWriter::SetEnvClassCounter(uint64_t counter)
{
    envClassCounter = counter;
}

void HashedASTWriter::SetStringLiteralCounter(uint64_t counter)
{
    stringLiteralCounter = counter;
}

void HashedASTWriter::SetCompileArgs(const std::vector<std::string>& args)
{
    for (const auto& it : std::as_const(args)) {
        (void)compileArgs.emplace_back(static_cast<TStringOffset>(builder.CreateSharedString(it)));
    }
}

using StringVecOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>>;
void HashedASTWriter::SetVarAndFuncDependency(
    const std::vector<std::pair<Ptr<const AST::Decl>, std::vector<Ptr<const AST::Decl>>>>& varAndFuncDep)
{
    std::map<std::string, const std::vector<Ptr<const AST::Decl>>*> orderedDep;
    for (auto& [decl, vecp] : std::as_const(varAndFuncDep)) {
        orderedDep.emplace(decl->rawMangleName, &vecp);
    }
    for (auto& [rawMangleName, vec] : std::as_const(orderedDep)) {
        std::set<std::string> strDeps;
        for (auto node : *vec) {
            (void)strDeps.emplace(node->rawMangleName);
        }
        auto depsIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(strDeps));
        auto declNameIdx = builder.CreateString(rawMangleName);
        (void)varAndFunc.emplace_back(CachedASTFormat::CreateDeclDep(builder, declNameIdx, depsIdx));
    }
}

void HashedASTWriter::SetCHIROptInfo(const OptEffectStrMap& optInfo)
{
    for (auto& it : optInfo) {
        auto declNameIdx = builder.CreateString(it.first);
        auto vecIdx = builder.CreateVectorOfStrings(Utils::SetToVec<std::string>(it.second));
        chirOptInfo.emplace_back(CachedASTFormat::CreateEffectMap(builder, declNameIdx, vecIdx));
    }
}

void HashedASTWriter::SetVirtualFuncDep(const VirtualWrapperDepMap& depMap)
{
    for (const auto& [raw, wrapper] : depMap) {
        auto rawOffset = builder.CreateSharedString(raw);
        auto wrapperOffset = builder.CreateSharedString(wrapper);
        virtualFuncDep.emplace_back(CachedASTFormat::CreateVirtualDep(builder, rawOffset, wrapperOffset));
    }
}

void HashedASTWriter::SetVarInitDep(const VarInitDepMap& depMap)
{
    for (const auto& [raw, wrapper] : depMap) {
        auto rawOffset = builder.CreateSharedString(raw);
        auto wrapperOffset = builder.CreateSharedString(wrapper);
        varInitDep.emplace_back(CachedASTFormat::CreateVirtualDep(builder, rawOffset, wrapperOffset));
    }
}

void HashedASTWriter::SetCCOutFuncs(const std::set<std::string>& funcs)
{
    for (const auto& func : std::as_const(funcs)) {
        (void)ccOutFuncs.emplace_back(static_cast<TStringOffset>(builder.CreateSharedString(func)));
    }
}

void HashedASTWriter::SetBitcodeFilesName(const std::vector<std::string>& bitcodeFiles)
{
    for (const auto& fileName : std::as_const(bitcodeFiles)) {
        (void)bitcodeFilesName.emplace_back(static_cast<TStringOffset>(builder.CreateSharedString(fileName)));
    }
}

std::vector<uint8_t> HashedASTWriter::AST2FB(const std::string& pkgName)
{
    auto codecVersion = builder.CreateString(CODIRA_VERSION);
    auto packageName = builder.CreateString(pkgName);
    auto optArgs = builder.CreateVector<TStringOffset>(compileArgs);
    auto declDep = builder.CreateVector<TDeclDepOffset>(varAndFunc);
    auto optInfo = builder.CreateVector<TEffectMapOffset>(chirOptInfo);
    auto virDep = builder.CreateVector<TVirtualDepOffset>(virtualFuncDep);
    auto varDep = builder.CreateVector<TVirtualDepOffset>(varInitDep);
    auto bcFilesName = builder.CreateVector<TStringOffset>(bitcodeFilesName);
    auto ccOutFuncVec = builder.CreateVector<TStringOffset>(ccOutFuncs);
    auto root = CachedASTFormat::CreateHashedPackage(builder, codecVersion, packageName, specs, optArgs, allAST,
        importedDecls, declDep, optInfo, virDep, varDep, ccOutFuncVec, lambdaCounter, envClassCounter,
        stringLiteralCounter, semaUsages, bcFilesName);
    FinishHashedPackageBuffer(builder, root);
    auto size = static_cast<size_t>(builder.GetSize());
    std::vector<uint8_t> data;
    data.resize(size);
    uint8_t* buf = builder.GetBufferPointer();
    CODEC_NULLPTR_CHECK(buf);
    std::copy(buf, buf + size, data.begin());
    return data;
}

bool HashedASTLoader::VerifyData()
{
    // We need to verify the size first.
    flatbuffers::Verifier verifier(serializedData.data(), serializedData.size(), FB_MAX_DEPTH, FB_MAX_TABLES);
    return CachedASTFormat::VerifyHashedPackageBuffer(verifier);
}

std::pair<bool, CompilationCache> HashedASTLoader::DeserializeData(const RawMangled2DeclMap& mangledName2DeclMap)
{
    if (!VerifyData()) {
        return {false, {}};
    }
    const auto package = CachedASTFormat::GetHashedPackage(serializedData.data());
    CODEC_NULLPTR_CHECK(package);
    CODEC_NULLPTR_CHECK(package->version());
    if (package->version()->str() != CODIRA_VERSION) {
        // Incremental compilation do not use cached data created with different version.
        return {false, {}};
    }
    CompilationCache cached;
    if (package->compileOptionArgs()) {
        for (uoffset_t i = 0; i < package->compileOptionArgs()->size(); i++) {
            auto arg = package->compileOptionArgs()->Get(i);
            (void)cached.compileArgs.emplace_back(arg->str());
        }
    }
    if (package->bitcodeFilesName()) {
        for (uoffset_t i = 0; i < package->bitcodeFilesName()->size(); i++) {
            auto fileName = package->bitcodeFilesName()->Get(i);
            (void)cached.bitcodeFilesName.emplace_back(fileName->str());
        }
    }
    cached.curPkgASTCache = LoadCachedAST(*package);
    for (std::move_iterator it{fileMap.begin()}; it != std::move_iterator{fileMap.end()}; ++it) {
        auto&& m{*it};
        // sort by gvid
        std::sort(m.second.begin(), m.second.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
        std::vector<std::string> tmp{};
        for (auto& mangledName : m.second) {
            tmp.push_back(std::move(mangledName.first));
        }
        cached.fileMap.emplace(std::move(const_cast<std::string&>(m.first)), std::move(tmp));
    }
    cached.importedASTCache = LoadImported(*package);
    cached.specs = package->specs();
    cached.lambdaCounter = package->lambdaCounter();
    cached.envClassCounter = package->envClassCounter();
    cached.stringLiteralCounter = package->strLitCounter();
    cached.semaInfo = LoadSemanticInfos(*package, mangledName2DeclMap);
    LoadCHIROptInfo(*package, mangledName2DeclMap, cached);
    LoadVirtualFuncDep(*package, cached);
    LoadVarInitDep(*package, cached);
    LoadCCOutFunc(*package, cached);
    return {true, std::move(cached)};
}

MemberDeclCache HashedASTLoader::Load(const MemberDecl& decl)
{
    MemberDeclCache res{};
    res.sigHash = decl.sig();
    res.srcUse = decl.srcUse();
    res.bodyHash = decl.body();
    res.astKind = decl.type();
    CODEC_NULLPTR_CHECK(decl.gvid());
    CODEC_NULLPTR_CHECK(decl.gvid()->file());
    res.gvid = {decl.gvid()->file()->str(), decl.gvid()->id()};
    res.isGV = decl.isGV();
    if (decl.members()) {
        for (const auto& member : *decl.members()) {
            res.members.emplace_back(Load(*member));
        }
    }
    res.rawMangle = decl.mangle()->str();
    res.cgMangle = decl.cgMangle()->str();
    return res;
}

static bool IsOOEAffectedDecl(const MemberDeclCache& decl)
{
    return decl.astKind == static_cast<uint8_t>(ASTKind::VAR_DECL) ||
        decl.astKind == static_cast<uint8_t>(ASTKind::PRIMARY_CTOR_DECL) ||
        decl.astKind == static_cast<uint8_t>(ASTKind::FUNC_DECL);
}

static bool IsOOEAffectedDecl(const TopLevelDeclCache& decl)
{
    return decl.astKind == static_cast<uint8_t>(ASTKind::VAR_DECL) ||
        decl.astKind == static_cast<uint8_t>(ASTKind::VAR_WITH_PATTERN_DECL) ||
        decl.astKind == static_cast<uint8_t>(ASTKind::FUNC_DECL) ||
        decl.astKind == static_cast<uint8_t>(ASTKind::MAIN_DECL) ||
        decl.astKind == static_cast<uint8_t>(ASTKind::MACRO_DECL);
}

// param `srcPkg`: true if is src package decl
TopLevelDeclCache HashedASTLoader::Load(const TopDecl& decl, bool srcPkg)
{
    TopLevelDeclCache res{};
    res.sigHash = decl.sig();
    res.srcUse = decl.srcUse();
    res.bodyHash = decl.body();
    res.astKind = decl.type();
    res.instVarHash = decl.instVar();
    res.virtHash = decl.virt();
    res.astKind = decl.type();
    if (srcPkg) {
        CODEC_NULLPTR_CHECK(decl.gvid());
        CODEC_NULLPTR_CHECK(decl.gvid()->file());
        res.gvid = {decl.gvid()->file()->str(), decl.gvid()->id()};
        res.isGV = decl.isGV();
    }
    if (decl.members()) {
        for (const auto& member : *decl.members()) {
            res.members.emplace_back(Load(*member));
        }

        if (srcPkg) {
            // push static vars first
            for (auto& member : res.members) {
                if (member.isGV) {
                    fileMap[member.gvid.file].emplace_back(member.rawMangle, member.gvid.id);
                }
            }
            // push other decls later, so in the filemap gvid increases
            for (auto& member : res.members) {
                // all members of members are property accessors, no gv
                for (auto& m1: member.members) {
                    fileMap[m1.gvid.file].emplace_back(m1.rawMangle, m1.gvid.id);
                }
                // redundant decls pushed here: enum constructors. They need not check against order change; however
                // on the one hand, collecting them does not affect the result as deleted decls are ignored. On the
                // other hand, current cache does not store this information, so they are not excluded here.
                if (IsOOEAffectedDecl(member) && !member.isGV) {
                    fileMap[member.gvid.file].emplace_back(member.rawMangle, member.gvid.id);
                }
            }
        }
    }
    
    if (decl.extends()) {
        for (const auto& e : *decl.extends()) {
            res.extends.emplace_back(e->str());
        }
    }
    res.cgMangle = decl.cgMangle()->str();
    return res;
}

ASTCache HashedASTLoader::LoadCachedAST(const HashedPackage& p)
{
    ASTCache ret{};
    if (p.topDecls()) {
        std::optional<std::string> last{};
        for (const auto& t : *p.topDecls()) {
            CODEC_NULLPTR_CHECK(t->gvid());
            CODEC_NULLPTR_CHECK(t->gvid()->file());
            auto r = Load(*t, true);
            if (IsOOEAffectedDecl(r) && !(r.astKind == static_cast<uint8_t>(ASTKind::VAR_DECL) && !r.isGV)) {
                fileMap[t->gvid()->file()->str()].emplace_back(t->mangle()->str(), t->gvid()->id());
            }
            // then load the cache of the decl
            ret.emplace(t->mangle()->str(), std::move(r));
        }
    }
    return ret;
}

std::unordered_map<RawMangledName, TopLevelDeclCache> HashedASTLoader::LoadImported(const HashedPackage& p)
{
    std::unordered_map<RawMangledName, TopLevelDeclCache> ret{};
    if (p.importedDecls()) {
        for (const auto& t : *p.importedDecls()) {
            ret.emplace(t->mangle()->str(), Load(*t, false));
        }
    }
    return ret;
}
