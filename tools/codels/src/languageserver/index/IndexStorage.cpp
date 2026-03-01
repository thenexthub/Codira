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

#include "IndexStorage.h"

#include <algorithm>
#include <fstream>
#include <ios>
#include <regex>
#include <string>
#include <utility>

#include "../Options.h"
#include "../common/FileStore.h"
#include "IndexDatabase.h"

namespace ark {
namespace lsp {
namespace {
std::pair<std::string, std::string> SplitFileName(const std::string &file)
{
    std::pair<std::string, std::string> res;
    auto found = file.find_last_of(".");
    if (found == std::string::npos) {
        return res;
    }
    std::string temp = file.substr(0, found);
    if (found = temp.find_last_of("."), found != std::string::npos) {
        res.second = temp.substr(found + 1);
        res.first = temp.substr(0, found);
    }
    return res;
}

std::string MergeFileName(const std::string& fullPkgName, const std::string &hashCode,
                          const std::string &extension)
{
    return fullPkgName + "." + hashCode + "." + extension;
}

void ReadSymsCompletions(Symbol &res, const IdxFormat::Symbol *sym)
{
    if (sym->completion_items()) {
        for (const auto &item : *sym->completion_items()) {
            std::string label = item->label()->str();
            std::string insert = item->insert_text()->str();
            res.completionItems.push_back({label, insert});
        }
    }
}

AST::Comment convertComment(const IdxFormat::Comment &comment)
{
    return AST::Comment{
        .style = static_cast<CommentStyle>(comment.style()),
        .kind = static_cast<AST::CommentKind>(comment.kind()),
        .info = [&]() {
            Token token = Token(TokenKind::COMMENT);
            token.SetValue(comment.info()->value()->str());
            return token;
        }()
    };
}

CommentGroup convertCommentGroup(const IdxFormat::CommentGroup &fbGroup)
{
    if (!fbGroup.cms()) {
        return {};
    }

    CommentGroup group;
    for (auto fbCommentOffset : *fbGroup.cms()) {
        auto &fbComment = *fbCommentOffset;
        AST::Comment comment = convertComment(fbComment);
        group.cms.push_back(comment);
    }
    return group;
}

void ReadSymsComments(Symbol &res, const IdxFormat::Symbol *sym)
{
    if (sym == nullptr || sym->comments() == nullptr) {
        return;
    }
    if (auto leadingComments = sym->comments()->leading_comments()) {
        std::for_each(leadingComments->begin(), leadingComments->end(),
            [&](const auto *fbGroup) { res.comments.leadingComments.push_back(convertCommentGroup(*fbGroup)); });
    }
    if (auto innerComments = sym->comments()->inner_comments()) {
        std::for_each(innerComments->begin(), innerComments->end(),
            [&](const auto *fbGroup) { res.comments.innerComments.push_back(convertCommentGroup(*fbGroup)); });
    }
    if (auto trailingComments = sym->comments()->trailing_comments()) {
        std::for_each(trailingComments->begin(), trailingComments->end(),
            [&](const auto *fbGroup) { res.comments.trailingComments.push_back(convertCommentGroup(*fbGroup)); });
    }
}

void ReadSymbolLocation(Symbol &res, const IdxFormat::Symbol *sym)
{
    if (sym->location()->begin() != nullptr) {
        res.location.begin.fileID = sym->location()->begin()->file_id();
        res.location.begin.line = sym->location()->begin()->line();
        res.location.begin.column = sym->location()->begin()->column();
    }
    if (sym->location()->end() != nullptr) {
        res.location.end.fileID = sym->location()->end()->file_id();
        res.location.end.line = sym->location()->end()->line();
        res.location.end.column = sym->location()->end()->column();
    }
    if (sym->location()->file_uri() != nullptr) {
        res.location.fileUri = sym->location()->file_uri()->str();
    }
}

void ReadSymbolDeclaration(Symbol &res, const IdxFormat::Symbol *sym)
{
    if (sym->declaration()->begin() != nullptr) {
        res.declaration.begin.fileID = sym->declaration()->begin()->file_id();
        res.declaration.begin.line = sym->declaration()->begin()->line();
        res.declaration.begin.column = sym->declaration()->begin()->column();
    }
    if (sym->declaration()->end() != nullptr) {
        res.declaration.end.fileID = sym->declaration()->end()->file_id();
        res.declaration.end.line = sym->declaration()->end()->line();
        res.declaration.end.column = sym->declaration()->end()->column();
    }
    if (sym->declaration()->file_uri() != nullptr) {
        res.declaration.fileUri = sym->declaration()->file_uri()->str();
    }
}

void ReadSymbolMacro(Symbol &res, const IdxFormat::Symbol *sym)
{
    if (sym->cur_macro_call()->begin() != nullptr) {
        res.curMacroCall.begin.fileID = sym->cur_macro_call()->begin()->file_id();
        res.curMacroCall.begin.line = sym->cur_macro_call()->begin()->line();
        res.curMacroCall.begin.column = sym->cur_macro_call()->begin()->column();
    }
    if (sym->cur_macro_call()->end() != nullptr) {
        res.curMacroCall.end.fileID = sym->cur_macro_call()->end()->file_id();
        res.curMacroCall.end.line = sym->cur_macro_call()->end()->line();
        res.curMacroCall.end.column = sym->cur_macro_call()->end()->column();
    }
    if (sym->cur_macro_call()->file_uri() != nullptr) {
        res.curMacroCall.fileUri = sym->cur_macro_call()->file_uri()->str();
    }
}

void ReadSymbol(Symbol &res, const IdxFormat::Symbol *sym)
{
    res.id = sym->id();
    if (sym->name() != nullptr) {
        res.name = sym->name()->str();
    }
    if (sym->scope() != nullptr) {
        res.scope = sym->scope()->str();
    }
    if (sym->location() != nullptr) {
        ReadSymbolLocation(res, sym);
    }
    if (sym->declaration() != nullptr) {
        ReadSymbolDeclaration(res, sym);
    }
    if (sym->cur_macro_call() != nullptr) {
        ReadSymbolMacro(res, sym);
    }
    res.kind = AST::ASTKind(sym->kind());
    if (sym->signature() != nullptr) {
        res.signature = sym->signature()->str();
    }
    if (sym->return_type() != nullptr) {
        res.returnType = sym->return_type()->str();
    }
    res.isMemberParam = sym->is_member_param();
    res.modifier = Modifier(sym->modifier());
    res.isCodeoSym = sym->is_codeo_sym();
    res.isDeprecated = sym->is_deprecated();
    if (sym->cur_module()) {
        res.curModule = sym->cur_module()->str();
    }
    ReadSymsCompletions(res, sym);
    ReadSymsComments(res, sym);
    if (sym->syscap() != nullptr) {
        res.syscap = sym->syscap()->str();
    }
    res.pkgModifier = Modifier(sym->pkg_modifier());
}

void ReadRefLocation(Ref &res, const IdxFormat::Ref *ref)
{
    if (ref->location()->begin() != nullptr) {
        res.location.begin.fileID = ref->location()->begin()->file_id();
        res.location.begin.line = ref->location()->begin()->line();
        res.location.begin.column = ref->location()->begin()->column();
    }
    if (ref->location()->end() != nullptr) {
        res.location.end.fileID = ref->location()->end()->file_id();
        res.location.end.line = ref->location()->end()->line();
        res.location.end.column = ref->location()->end()->column();
    }
    if (ref->location()->file_uri() != nullptr) {
        res.location.fileUri = ref->location()->file_uri()->str();
    }
}

void ReadRef(Ref &res, const IdxFormat::Ref *ref)
{
    if (ref->location() != nullptr) {
        ReadRefLocation(res, ref);
    }
    res.kind = RefKind(ref->kind());
    res.container = ref->id();
    res.isCodeoRef = ref->is_codeo_ref();
}

void ReadRelation(Relation &res, const IdxFormat::Relation *relation)
{
    res.subject = relation->subject();
    res.predicate = RelationKind(relation->kind());
    res.object = relation->object();
}

void ReadCrossSymbolLocation(CrossSymbol &crossSymbol, const IdxFormat::CrossSymbol *crs)
{
    if (crs->location()->begin() != nullptr) {
        crossSymbol.location.begin.fileID = crs->location()->begin()->file_id();
        crossSymbol.location.begin.line = crs->location()->begin()->line();
        crossSymbol.location.begin.column = crs->location()->begin()->column();
    }
    if (crs->location()->end() != nullptr) {
        crossSymbol.location.begin.fileID = crs->location()->begin()->file_id();
        crossSymbol.location.begin.line = crs->location()->begin()->line();
        crossSymbol.location.begin.column = crs->location()->begin()->column();
    }
    if (crs->location()->file_uri() != nullptr) {
        crossSymbol.location.fileUri = crs->location()->file_uri()->str();
    }
}

void ReadCrossSymbol(CrossSymbol &crossSymbol, const IdxFormat::CrossSymbol *crs)
{
    crossSymbol.id = crs->id();
    if (crs->name() != nullptr) {
        crossSymbol.name = crs->name()->str();
    }
    crossSymbol.crossType = CrossType(crs->cross_type());
    if (crs->location() != nullptr) {
        ReadCrossSymbolLocation(crossSymbol, crs);
    }
    crossSymbol.container = crs->container();
    if (crs->container_name() != nullptr) {
        crossSymbol.containerName = crs->container_name()->str();
    }
}

flatbuffers::Offset<IdxFormat::CommentGroups> CreateFBCommentGroups(
    flatbuffers::FlatBufferBuilder &builder, const CommentGroups &comments)
{
    auto convertGroup = [&builder](const std::vector<CommentGroup> &groups) {
        std::vector<flatbuffers::Offset<IdxFormat::CommentGroup>> fbGroups;
        for (const auto &group : groups) {
            std::vector<flatbuffers::Offset<IdxFormat::Comment>> fbComments;
            for (const auto &comment : group.cms) {
                auto value = builder.CreateString(comment.info.Value());
                auto token = IdxFormat::CreateToken(builder, value);
                auto fbComment = CreateComment(
                    builder,
                    {},
                    {},
                    token
                );
                fbComments.push_back(fbComment);
            }
            auto fbCms = builder.CreateVector(fbComments);
            fbGroups.push_back(CreateCommentGroup(builder, fbCms));
        }
        return builder.CreateVector(fbGroups);
    };

    auto lead = convertGroup(comments.leadingComments);
    auto inner = convertGroup(comments.innerComments);
    auto trail = convertGroup(comments.trailingComments);

    return CreateCommentGroups(builder, lead, inner, trail);
}

auto StoreSymbol(flatbuffers::FlatBufferBuilder &builder, const Symbol &sym)
{
    auto name = builder.CreateString(sym.name);
    auto scope = builder.CreateString(sym.scope);
    auto begin = IdxFormat::Position(sym.location.begin.fileID, sym.location.begin.line,
                                     sym.location.begin.column);
    auto end = IdxFormat::Position(sym.location.end.fileID, sym.location.end.line,
                                   sym.location.end.column);
    auto uri = builder.CreateString(sym.location.fileUri);
    auto loc = IdxFormat::CreateLocation(builder, &begin, &end, uri);
    auto decl_begin = IdxFormat::Position(sym.declaration.begin.fileID, sym.declaration.begin.line,
                                          sym.declaration.begin.column);
    auto decl_end = IdxFormat::Position(sym.declaration.end.fileID, sym.declaration.end.line,
                                        sym.declaration.end.column);
    auto decl_uri = builder.CreateString(sym.declaration.fileUri);
    auto decl_loc = IdxFormat::CreateLocation(builder, &decl_begin, &decl_end, decl_uri);
    auto sig = builder.CreateString(sym.signature);
    auto ret = builder.CreateString(sym.returnType);
    auto module = builder.CreateString(sym.curModule);
    auto macro_call_begin = IdxFormat::Position(sym.curMacroCall.begin.fileID,
                                                sym.curMacroCall.begin.line, sym.curMacroCall.begin.column);
    auto macro_call_end = IdxFormat::Position(sym.curMacroCall.end.fileID,
                                              sym.curMacroCall.end.line, sym.curMacroCall.end.column);
    auto macro_call_uri = builder.CreateString(sym.curMacroCall.fileUri);
    auto macro = IdxFormat::CreateLocation(builder, &macro_call_begin,
                                           &macro_call_end, macro_call_uri);
    std::vector<flatbuffers::Offset<IdxFormat::CompletionItem>> completion_vec;
    for (const auto& [label, insert_text] : sym.completionItems) {
        auto fb_label = builder.CreateString(label);
        auto fb_insert = builder.CreateString(insert_text);
        auto completion_item =
            IdxFormat::CreateCompletionItem(builder, fb_label, fb_insert);
        completion_vec.push_back(completion_item);
    }
    auto completion_items = builder.CreateVector(completion_vec);
    auto comments = CreateFBCommentGroups(builder, sym.comments);
    auto syscap = builder.CreateString(sym.syscap);
    return IdxFormat::CreateSymbol(builder, sym.id, name, scope, loc, decl_loc,
                                   static_cast<uint16_t>(sym.kind), sig, ret, sym.isMemberParam,
                                   static_cast<uint8_t>(sym.modifier), sym.isCodeoSym, sym.isDeprecated,
                                   module, macro, completion_items, comments, syscap,
                                   static_cast<uint8_t>(sym.pkgModifier));
}

auto StoreRef(flatbuffers::FlatBufferBuilder &builder, const Ref &ref)
{
    auto begin = IdxFormat::Position(ref.location.begin.fileID, ref.location.begin.line,
                                     ref.location.begin.column);
    auto end = IdxFormat::Position(ref.location.end.fileID, ref.location.end.line,
                                   ref.location.end.column);
    auto uri = builder.CreateString(ref.location.fileUri);
    auto loc = IdxFormat::CreateLocation(builder, &begin, &end, uri);
    return IdxFormat::CreateRef(builder, loc, static_cast<uint16_t>(ref.kind), ref.container, ref.isCodeoRef);
}

auto StoreExtend(flatbuffers::FlatBufferBuilder &builder, const ExtendItem &extendItem)
{
    auto interfaceName = builder.CreateString(extendItem.interfaceName);
    return IdxFormat::CreateExtend(builder, extendItem.id,
        static_cast<uint8_t>(extendItem.modifier), interfaceName);
}

auto StoreRelation(flatbuffers::FlatBufferBuilder &builder, const Relation &re)
{
    return IdxFormat::CreateRelation(builder, re.subject, static_cast<uint16_t>(re.predicate),
                                     re.object);
}

auto StoreCrossSymbol(flatbuffers::FlatBufferBuilder &builder, const CrossSymbol &crs)
{
    auto name = builder.CreateString(crs.name);
    auto begin = IdxFormat::Position(crs.location.begin.fileID, crs.location.begin.line,
                                     crs.location.begin.column);
    auto end = IdxFormat::Position(crs.location.end.fileID, crs.location.end.line,
                                   crs.location.end.column);
    auto uri = builder.CreateString(crs.location.fileUri);
    auto loc = CreateLocation(builder, &begin, &end, uri);
    auto containerName = builder.CreateString(crs.containerName);
    return CreateCrossSymbol(builder, crs.id, name, static_cast<uint8_t>(crs.crossType), loc,
        crs.container, containerName);
}
} // namespace

std::optional<std::unique_ptr<FileIn>> AstFileHandler::LoadShard(std::string filePath) const
{
    auto in = std::make_unique<AstFileIn>();
    std::string reason;
    bool ret = FileUtil::ReadBinaryFileToBuffer(filePath, in->data, reason);
    if (!ret) {
        Trace::Elog(reason);
        return std::nullopt;
    }
    return std::move(in);
}

void AstFileHandler::StoreShard(std::string filePath, const FileOut *out) const
{
    auto fileOut = dynamic_cast<const AstFileOut *>(out);
    if (!fileOut || !fileOut->data) {
        return;
    }
    bool ret = FileUtil::WriteBufferToASTFile(filePath, *fileOut->data);
    if (!ret) {
        Trace::Log("ast file write");
    }
}

void CacheManager::InitDir()
{
    std::string cacheRoot = FileUtil::JoinPath(basePath, ".cache");
    astdataDir = FileUtil::JoinPath(cacheRoot, "astdata/");
    indexDir = FileUtil::JoinPath(cacheRoot, "index/");
    if (!FileUtil::FileExist(astdataDir)) {
        auto ret = FileUtil::CreateDirs(astdataDir);
        if (ret == -1) {
            return;
        }
    }

    if (!FileUtil::FileExist(indexDir)) {
        auto ret = FileUtil::CreateDirs(indexDir);
        if (ret == -1) {
            return;
        }
    }

    for (const auto &iter : FileUtil::GetAllFilesUnderCurrentPath(astdataDir, "ast")) {
        (void)astIdMap.emplace(SplitFileName(iter));
    }
}

bool CacheManager::IsStale(const std::string &pkgName, const std::string &digest)
{
    auto found = astIdMap.find(pkgName);
    // Package do not have cache in disk.
    if (found == astIdMap.end()) {
        return true;
    }
    // Cache is fresh, not to rebuild.
    if (found->second == digest) {
        return false;
    }

    auto staleFileName = MergeFileName(pkgName, found->second, "ast");
    auto staleFilePath = FileUtil::JoinPath(astdataDir, staleFileName);
    // Cache is stale, delete cache file and rebuild package.
    if (FileUtil::FileExist(staleFilePath)) {
        (void)FileUtil::Remove(staleFilePath);
    }
    return true;
}

void CacheManager::UpdateIdMap(const std::string &pkgName, const std::string &digest)
{
    std::lock_guard<std::mutex> lock(cacheMtx);
    astIdMap.insert_or_assign(pkgName, digest);
}

std::optional<std::unique_ptr<FileIn>> CacheManager::Load(const std::string &pkgName)
{
    auto found = astIdMap.find(pkgName);
    if (found == astIdMap.end()) {
        return std::nullopt;
    }
    std::string fileName = MergeFileName(pkgName, found->second, "ast");
    std::string filePath = FileUtil::JoinPath(astdataDir, fileName);
    return astLoader->LoadShard(filePath);
}

void CacheManager::Store(const std::string &pkgName, const std::string &digest, const std::vector<uint8_t> &buffer)
{
    if (digest.empty() || Options::GetInstance().IsOptionSet("--test")) {
        return;
    }
    lsp::AstFileOut out;
    out.shardId = digest;
    out.data = &buffer;

    auto found = astIdMap.find(pkgName);
    if (found != astIdMap.end()) {
        auto staleFileName = MergeFileName(pkgName, found->second, "ast");
        auto staleFilePath = FileUtil::JoinPath(astdataDir, staleFileName);
        if (FileUtil::FileExist(staleFilePath)) {
            (void)FileUtil::Remove(staleFilePath);
        }
    }
    UpdateIdMap(pkgName, digest);
    std::string fileName = MergeFileName(pkgName, digest, "ast");
    std::string filePath = FileUtil::JoinPath(astdataDir, fileName);
    if (!filePath.empty()) {
        astLoader->StoreShard(filePath, &out);
    }
}

std::optional<std::unique_ptr<IndexFileIn>> CacheManager::LoadIndexShard(const std::string &curPkgName,
                                                                         const std::string &shardIdentifier) const
{
    std::string idxFilePath = GetShardPathFromFilePath(curPkgName, shardIdentifier);
    std::vector<uint8_t> buffer;
    std::string reason;

    (void)FileUtil::ReadBinaryFileToBuffer(idxFilePath, buffer, reason);

    flatbuffers::Verifier verifier = flatbuffers::Verifier(buffer.data(), buffer.size());
    bool ok = IdxFormat::VerifyHashedPackageBuffer(verifier);
    if (!ok) {
        (void)FileUtil::Remove(idxFilePath);
        return std::nullopt;
    }

    auto ifi = std::make_unique<IndexFileIn>();
    auto hashedPackage = IdxFormat::GetHashedPackage(buffer.data());
    if (hashedPackage == nullptr) {
        return std::nullopt;
    }
    // read symbols
    auto symbolSlab = hashedPackage->symbol_slab();
    if (symbolSlab != nullptr) {
        for (flatbuffers::uoffset_t i = 0; i < symbolSlab->size(); ++i) {
            auto sym = symbolSlab->Get(i);
            Symbol res;
            ReadSymbol(res, sym);
            (void)ifi->symbols.emplace_back(res);
        }
    }

    // read refs
    readRefs(*hashedPackage, ifi);

    // read relations
    auto relationSlab = hashedPackage->relation_slab();
    if (relationSlab != nullptr) {
        for (flatbuffers::uoffset_t i = 0; i < relationSlab->size(); ++i) {
            auto relation = relationSlab->Get(i);
            Relation res;
            ReadRelation(res, relation);
            (void)ifi->relations.emplace_back(res);
        }
    }

    // read extends
    readExtends(*hashedPackage, ifi);

    // read cross
    readCrossSymbols(*hashedPackage, ifi);
    return std::move(ifi);
}

void CacheManager::readRefs(
    const IdxFormat::HashedPackage &package, std::unique_ptr<ark::lsp::IndexFileIn> &ifi) const
{
    auto refSlab = package.ref_slab();
    if (refSlab == nullptr) {
        return;
    }
    for (flatbuffers::uoffset_t i = 0; i < refSlab->size(); ++i) {
        std::pair<SymbolID, std::vector<Ref>> resPair;
        auto sym2Ref = refSlab->Get(i);
        resPair.first = sym2Ref->id();
        auto refs = sym2Ref->refs();
        if (refs != nullptr) {
            for (flatbuffers::uoffset_t j = 0; j < refs->size(); ++j) {
                auto ref = refs->Get(j);
                Ref res;
                ReadRef(res, ref);
                (void)resPair.second.emplace_back(res);
            }
        }
        (void)ifi->refs.emplace(resPair);
    }
}

void CacheManager::readExtends(
    const IdxFormat::HashedPackage &package, std::unique_ptr<ark::lsp::IndexFileIn> &ifi) const
{
    auto extendSlab = package.extend_slab();
    if (extendSlab != nullptr) {
        for (flatbuffers::uoffset_t i = 0; i < extendSlab->size(); ++i) {
            auto extendItem = extendSlab->Get(i);
            if (!extendItem) {
                continue;
            }
            std::pair<SymbolID, std::vector<ExtendItem>> extendPair;
            extendPair.first = extendItem->id();
            auto extends = extendItem->extends();
            if (extends == nullptr) {
                continue;
            }
            for (flatbuffers::uoffset_t j = 0; j < extends->size(); ++j) {
                auto extend = extends->Get(j);
                ExtendItem res = {.id = extend->id(),
                                  .modifier = Modifier(extend->modifier()),
                                  .interfaceName = extend->interface() ? extend->interface()->str() : ""};
                (void)extendPair.second.emplace_back(res);
            }
            (void)ifi->extends.emplace(extendPair);
        }
    }
}

void CacheManager::readCrossSymbols(
        const IdxFormat::HashedPackage &package, std::unique_ptr<ark::lsp::IndexFileIn> &ifi) const
{
    auto crossSymbolSlab = package.cross_symbol_slab();
    if (crossSymbolSlab == nullptr) {
        return;
    }
    for (flatbuffers::uoffset_t i = 0; i < crossSymbolSlab->size(); i++) {
        auto crs = crossSymbolSlab->Get(i);
        if (!crs) {
            continue;
        }
        CrossSymbol crossSymbol;
        ReadCrossSymbol(crossSymbol, crs);
        (void)ifi->crossSymbos.emplace_back(crossSymbol);
    }
}

void CacheManager::StoreIndexShard(const std::string &curPkgName, const std::string &shardIdentifier,
                                   const IndexFileOut &shard) const
{
    auto found = astIdMap.find(curPkgName);
    if (found != astIdMap.end()) {
        auto staleFileName = MergeFileName(curPkgName, found->second, "idx");
        auto staleFilePath = FileUtil::JoinPath(indexDir, staleFileName);
        if (FileUtil::FileExist(staleFilePath)) {
            (void)FileUtil::Remove(staleFilePath);
        }
    }

    std::string idxFilePath =
        FileUtil::Normalize(GetShardPathFromFilePath(curPkgName, shardIdentifier));
    flatbuffers::FlatBufferBuilder builder;

    // serialize symbols
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbolVec;
    for (const auto &sym : *shard.symbols) {
        symbolVec.push_back(StoreSymbol(builder, sym));
    }
    auto symbolSlab = builder.CreateVector(symbolVec);

    // serialize refs
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Ref>> sym2RefVec;
    for (const auto &pair : *shard.refs) {
        std::vector<flatbuffers::Offset<IdxFormat::Ref>> refVec;
        for (const auto &ref : pair.second) {
            refVec.push_back(StoreRef(builder, ref));
        }
        auto refs = builder.CreateVector(refVec);
        auto sym2Ref = IdxFormat::CreateSym2Ref(builder, pair.first, refs);
        sym2RefVec.push_back(sym2Ref);
    }
    auto refSlab = builder.CreateVector(sym2RefVec);

    // serialize relations
    std::vector<flatbuffers::Offset<IdxFormat::Relation>> relationVec;
    for (const auto &re : *shard.relations) {
        (void)relationVec.emplace_back(StoreRelation(builder, re));
    }
    auto relationSlab = builder.CreateVector(relationVec);

    // serialize symbol with extends
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Extend>> sym2ExtendVec;
    for (const auto &pair : *shard.extends) {
        std::vector<flatbuffers::Offset<IdxFormat::Extend>> extendVec;
        for (const auto &extend : pair.second) {
            extendVec.push_back(StoreExtend(builder, extend));
        }
        auto extends = builder.CreateVector(extendVec);
        auto sym2Extend = IdxFormat::CreateSym2Extend(builder, pair.first, extends);
        sym2ExtendVec.push_back(sym2Extend);
    }
    auto extendSlab = builder.CreateVector(sym2ExtendVec);

    // serialize cross symbol
    std::vector<flatbuffers::Offset<IdxFormat::CrossSymbol>> crossSymbolVec;
    for (const auto &crs : *shard.crossSymbos) {
        crossSymbolVec.push_back(StoreCrossSymbol(builder, crs));
    }
    auto crossSymbolSlab = builder.CreateVector(crossSymbolVec);

    auto hashedPackage = CreateHashedPackage(builder, symbolSlab, refSlab, relationSlab, extendSlab, crossSymbolSlab);
    IdxFormat::FinishHashedPackageBuffer(builder, hashedPackage);

    std::ofstream outFile{FileStore::NormalizePath(idxFilePath).c_str(), std::ios::binary | std::ios::out};
    (void)outFile.write(reinterpret_cast<char *>(builder.GetBufferPointer()), builder.GetSize());
    outFile.close();
}

std::string CacheManager::GetShardPathFromFilePath(std::string curPkgName,
                                                   const std::string &shardIdentifier) const
{
    std::replace(curPkgName.begin(), curPkgName.end(), '/', '.');
    std::string idxFileName = curPkgName + "." + shardIdentifier + ".idx";
    auto sr = FileUtil::JoinPath(indexDir, idxFileName);
    return sr;
}
} // namespace lsp
} // namespace ark
