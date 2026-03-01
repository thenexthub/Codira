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

#include <random>
#include <fstream>
#include <iostream>
#include <optional>
#include "../CompilerCodiraProject.h"
#include "BackgroundIndexDB.h"

namespace ark {
namespace lsp {
BackgroundIndexDB::BackgroundIndexDB(IndexDatabase &indexDB)
    : db(indexDB) {}

BackgroundIndexDB::~BackgroundIndexDB() {}

void BackgroundIndexDB::UpdateFile(const std::map<int, std::vector<std::string>> &fileMap)
{
    db.Update([&](IndexDatabase::DBUpdate update) {
        for (auto cand: fileMap) {
            update.InsertFileWithId(cand.first, cand.second);
        }
        return update.Done();
    });
}

void BackgroundIndexDB::DeleteFiles(const std::unordered_set<std::string> &fileSet)
{
    db.Update([&](IndexDatabase::DBUpdate update) {
        for (const auto& file: fileSet) {
            update.DeleteFile(file);
        }
        return update.Done();
    });
}

std::string BackgroundIndexDB::GetFileDigest(uint32_t fileId)
{
    std::string digest;
    db.GetFileDigest(fileId, digest);
    return digest;
}

void BackgroundIndexDB::Update(const std::string &curPkgName, IndexFileOut index)
{
    db.Update([&](IndexDatabase::DBUpdate update) {
        Trace::Log("db update for codeo start");
        for (const Symbol &S : *index.symbols) {
            update.InsertSymbol(S);
            for (const auto &completionItem : S.completionItems) {
                update.InsertCompletion(S, completionItem);
            }
            const auto [leadCommentGroup, innerCommentGroup, trailCommentGroup] = S.comments;
            for (const auto &innerComment : innerCommentGroup) {
                for (const auto &comment : innerComment.cms) {
                    update.InsertComment(S, comment);
                }
            }
            for (auto &leadComment : leadCommentGroup) {
                for (const auto &comment : leadComment.cms) {
                    update.InsertComment(S, comment);
                }
            }
            for (auto &trailComment : trailCommentGroup) {
                for (const auto &comment : trailComment.cms) {
                    update.InsertComment(S, comment);
                }
            }
        }
        for (const auto &SymbolRefs : *index.refs) {
            for (const Ref &R : SymbolRefs.second) {
                update.InsertReference(GetArrayFromID(SymbolRefs.first), R);
            }
        }
        for (const Relation &R : *index.relations) {
            update.InsertRelation(GetArrayFromID(R.subject), R.predicate, GetArrayFromID(R.object));
        }
        for (const auto &extends : *index.extends) {
            for (const ExtendItem &extendItem : extends.second) {
                update.InsertExtend(GetArrayFromID(extends.first), GetArrayFromID(extendItem.id),
                    extendItem.modifier, extendItem.interfaceName, curPkgName);
            }
        }
        for (const CrossSymbol &crs : *index.crossSymbos) {
            update.InsertCrossSymbol(curPkgName, crs);
        }
        return update.Done();
    });
}

void BackgroundIndexDB::UpdateAll(const std::map<int, std::vector<std::string>> &fileMap,
    std::unique_ptr<MemIndex> index)
{
    db.Update([&](IndexDatabase::DBUpdate update) {
        for (auto cand: fileMap) {
            update.InsertFileWithId(cand.first, cand.second);
        }

        std::vector<Symbol> symbols;
        std::vector<std::pair<IDArray, CompletionItem>> completions;
        std::vector<std::pair<IDArray, Comment>> comments;
        // collect symbols
        for (const auto &item : index->pkgSymsMap) {
            auto codeoPkgName = item.first;
            for (const auto &sym : item.second) {
                if (sym.id == INVALID_SYMBOL_ID) {
                    continue;
                }
                // collect symbols
                symbols.push_back(sym);
                // collect completions
                for (const auto &completionItem: sym.completionItems) {
                    completions.emplace_back(sym.idArray, completionItem);
                }
                // collect comment
                std::vector<CommentGroup> commentGroups;
                const auto [leadCommentGroup, innerCommentGroup, trailCommentGroup] = sym.comments;
                commentGroups.insert(commentGroups.end(), leadCommentGroup.begin(), leadCommentGroup.end());
                commentGroups.insert(commentGroups.end(), innerCommentGroup.begin(), innerCommentGroup.end());
                commentGroups.insert(commentGroups.end(), trailCommentGroup.begin(), trailCommentGroup.end());
                for (const auto &commentGroup : commentGroups) {
                    for (const auto &cmt : commentGroup.cms) {
                        Comment comment;
                        comment.id = sym.id;
                        comment.style = cmt.style;
                        comment.kind = cmt.kind;
                        comment.commentStr = cmt.info.Value();
                        comments.emplace_back(sym.idArray, comment);
                    }
                }
            }
        }
        update.InsertSymbols(symbols);
        update.InsertCompletions(completions);
        update.InsertComments(comments);

        // collect refs
        std::vector<std::pair<IDArray, Ref>> refs;
        for (const auto& item : index->pkgRefsMap) {
            for (const auto &symbolRefs : item.second) {
                if (symbolRefs.first == INVALID_SYMBOL_ID) {
                    continue;
                }
                for (const auto &ref : symbolRefs.second) {
                    refs.emplace_back(GetArrayFromID(symbolRefs.first), ref);
                }
            }
        }
        update.InsertReferences(refs);

        // collect relations
        std::vector<Relation> relations;
        for (const auto& item : index->pkgRelationsMap) {
            relations.insert(relations.end(), item.second.begin(), item.second.end());
        }
        update.InsertRelations(relations);

        // collect extends
        std::map<std::pair<std::string, SymbolID>, std::vector<ExtendItem>> extends;
        for (const auto& item : index->pkgExtendsMap) {
            auto codeoPkgName = item.first;
            for (const auto &extend : item.second) {
                extends.insert_or_assign(std::make_pair(codeoPkgName, extend.first), extend.second);
            }
        }
        update.InsertExtends(extends);

        // collect cross language symbols
        std::vector<std::pair<std::string, CrossSymbol>> crsSymbols;
        for (const auto &item : index->pkgCrossSymsMap) {
            for (const auto &crs : item.second) {
                crsSymbols.emplace_back(item.first, crs);
            }
        }
        update.InsertCrossSymbols(crsSymbols);
        return update.Done();
    });
    index.reset(nullptr);
}

void BackgroundIndexDB::FuzzyFind(const FuzzyFindRequest &req,
    std::function<void(const Symbol &)> callback) const
{
    size_t count = 0;
    bool more = false;
    uint32_t limit = req.limit.value_or(std::numeric_limits<uint32_t>::max());
    db.GetMatchingSymbols(
        req.query,
        [&](const Symbol &sym) {
            if (!req.anyScope &&
               std::find(req.scopes.begin(), req.scopes.end(), sym.scope) != req.scopes.end()) {
                return true;
            }
            if (++count < limit) {
              callback(sym);
              return true;
            }
            more = true;
            return false;
        },
        (!req.anyScope && req.scopes.size() == 1)
                ? std::optional<std::string>(req.scopes[0]) : std::nullopt,
        req.restrictForCodeCompletion
                ? std::optional<Symbol::SymbolFlag>(Symbol::INDEXED_FOR_CODE_COMPLETION) : std::nullopt);
}

void BackgroundIndexDB::Refs(const RefsRequest &req,
    std::function<void(const Ref &)> callback) const
{
    for (const SymbolID &ID : req.ids) {
        db.GetReferences(ID, req.filter, [&](const Ref &ref) {
            callback(ref);
            return true;
        });
    }
}

void BackgroundIndexDB::RefsFindReference(const RefsRequest &req,
    Ref &definition, std::function<void(const Ref &)> callback) const
{
    for (const SymbolID &ID : req.ids) {
        db.GetReferences(ID, req.filter, [&](const Ref &ref) {
            if (ref.kind == RefKind::DEFINITION) {
                definition = ref;
            }
            callback(ref);
            return true;
        });
    }
}

void BackgroundIndexDB::FileRefs(const FileRefsRequest &req,
    std::function<void(const Ref &ref, const SymbolID symId)> callback) const
{
    db.GetFileReferences(req.fileUri, req.filter, [&](const Ref &ref, const SymbolID symId) {
        callback(ref, symId);
        return true;
    });
}

void BackgroundIndexDB::Lookup(const LookupRequest &req,
    std::function<void(const Symbol &)> callback) const
{
    for (const SymbolID &id : req.ids) {
        IDArray array = GetArrayFromID(id);
        db.GetSymbolByID(array, [&](Symbol sym) {
            callback(sym);
            return true;
        });
    }
}

void BackgroundIndexDB::GetExportSID(IDArray array,
                                     std::function<void(const CrossSymbol &)> callback) const
{
    db.GetCrossSymbolByID(array, callback);
}

void BackgroundIndexDB::FindPkgSyms(const PkgSymsRequest &req, std::function<void(const Symbol &)> callback) const
{
    db.GetPkgSymbols(req.fullPkgName, [&](Symbol sym) {
        callback(sym);
        return true;
    });
}

void BackgroundIndexDB::Relations(const RelationsRequest &req,
    std::function<void(const Relation &)> callback) const
{
    db.GetRelations(req.id, req.predicate, [&](const Relation &rel) {
        callback(rel);
        return true;
    });
}

void BackgroundIndexDB::GetFuncByName(const FuncSigRequest &req,
                                      std::function<void(const Symbol &)> callback) const
{
    db.GetSymbolsByName(req.funcName, [&](Symbol sym) {
        callback(sym);
        return true;
    });
}

Symbol BackgroundIndexDB::GetAimSymbol(const Decl& decl)
{
    auto pkgName = decl.fullPackageName;
    auto symbolID = GetArrayFromID(GetSymbolId(decl));
    std::vector<lsp::Symbol> syms;
    db.GetSymbolByID(symbolID, [&syms](const lsp::Symbol &sym) {
        syms.emplace_back(sym);
        return true;
    });
    if (!syms.empty()) {
        return *syms.begin();
    }
    return {};
}

void BackgroundIndexDB::Callees(const std::string &pkgName, const SymbolID &declId,
    std::function<void(const SymbolID &, const Ref &)> callback) const
{
    db.GetReferred(declId, [&](const SymbolID &declSymId, const Ref &ref) {
        callback(declSymId, ref);
    });
}

void BackgroundIndexDB::FindImportSymsOnCompletion(
    const std::pair<std::unordered_set<SymbolID>, std::unordered_set<SymbolID>>& filterSyms,
    const std::string &curPkgName, const std::string &curModule, const std::string &prefix,
    std::function<void(const std::string &, const Symbol &, const CompletionItem &)> callback)
{
    if (Options::GetInstance().IsOptionSet("test")) {
        return;
    }
    const auto &normalCompleteSyms  = filterSyms.first;
    const auto &importDeclSyms  = filterSyms.second;
    size_t normalCompleteCount = 0;
    size_t importDeclCount = 0;
    std::unordered_set<std::string> curModuleDeps =
        CompilerCodiraProject::GetInstance()->GetOneModuleDirectDeps(curModule);
    db.GetSymbolsAndCompletions(prefix, [&](const Symbol &sym, const CompletionItem &completion) {
        std::string symPackage;
        std::string symModule;
        db.GetFileWithUri(sym.location.fileUri, [&](std::string packageName, std::string moduleName) {
            symPackage = packageName;
            symModule = moduleName;
            return true;
        });
        // symModule info is empty for codeo sym.
        if (symPackage.empty()) {
            std::cerr << "FilterRepeatedSymbol: empty info for " << sym.name;
            std::cerr << " in <" << symPackage << ", " << symModule << "\n";
        }
        if (curPkgName == symPackage) {
            return true;
        }
        // filter symbols that not dependent by curModule
        if (!sym.isCodeoSym && !curModuleDeps.count(symModule)) {
            return true;
        }
        auto relation = GetPackageRelation(curPkgName, symPackage);
        bool isPkgAccess =
            sym.pkgModifier == Modifier::PUBLIC
            || (relation == PackageRelation::CHILD && (sym.pkgModifier == Modifier::INTERNAL
                                                          || sym.pkgModifier == Modifier::PROTECTED))
            || (relation == PackageRelation::SAME_MODULE && sym.pkgModifier == Modifier::PROTECTED)
            || (relation == PackageRelation::PARENT && sym.pkgModifier == Modifier::PROTECTED);
        if (!isPkgAccess) {
            return true;
        }
        // filter symbols that are already in the NormalComplete
        if (sym.id != 0 && normalCompleteCount < normalCompleteSyms.size() && normalCompleteSyms.count(sym.id)) {
            normalCompleteCount++;
            return true;
        }
        // filter imported syms
        if (importDeclCount < importDeclSyms.size() && importDeclSyms.count(sym.id)) {
            importDeclCount++;
            return true;
        }
        // filter not top decl
        if (sym.scope.find(':') != std::string::npos) {
            return true;
        }
        // filter by modifier
        bool isAccessible =
            sym.modifier == Modifier::PUBLIC
            || (relation == PackageRelation::CHILD && (sym.modifier == Modifier::INTERNAL
                                                       || sym.modifier == Modifier::PROTECTED))
            || (relation == PackageRelation::SAME_MODULE && sym.modifier == Modifier::PROTECTED)
            || (relation == PackageRelation::PARENT && sym.modifier == Modifier::PROTECTED);
        // filter root pkg symbols if cur module is combined
        if (isAccessible
            && !CompilerCodiraProject::GetInstance()->IsCombinedSym(curModule, curPkgName, symPackage)) {
            callback(symPackage, sym, completion);
        }
        return true;
    });
}

void BackgroundIndexDB::FindImportSymsOnQuickFix(const std::string &curPkgName, const std::string &curModule,
    const std::unordered_set<SymbolID> &importDeclSyms,
    const std::string& identifier, const std::function<void(const std::string &, const Symbol &)>& callback)
{
    size_t importDeclCount = 0;
    std::unordered_set<std::string> curModuleDeps =
        CompilerCodiraProject::GetInstance()->GetOneModuleDirectDeps(curModule);
    db.GetSymbolsByName(identifier, [&](const Symbol &sym) {
        std::string symPackage;
        std::string symModule;
        db.GetFileWithUri(sym.location.fileUri, [&](std::string packageName, std::string moduleName) {
            symPackage = packageName;
            symModule = moduleName;
            return true;
        });
        // symModule info is empty for codeo sym.
        if (symPackage.empty()) {
            std::cerr << "FilterRepeatedSymbol: empty info for " << sym.name;
            std::cerr << " in <" << symPackage << ", " << symModule << "\n";
        }
        if (curPkgName == symPackage) {
            return true;
        }
        // filter symbols that not dependent by curModule
        if (!sym.isCodeoSym && !curModuleDeps.count(symModule)) {
            return true;
        }
        auto relation = GetPackageRelation(curPkgName, symPackage);
        bool isPkgAccess =
            sym.pkgModifier == Modifier::PUBLIC
            || (relation == PackageRelation::CHILD && (sym.pkgModifier == Modifier::INTERNAL
                                                          || sym.pkgModifier == Modifier::PROTECTED))
            || (relation == PackageRelation::SAME_MODULE && sym.pkgModifier == Modifier::PROTECTED)
            || (relation == PackageRelation::PARENT && sym.pkgModifier == Modifier::PROTECTED);
        if (!isPkgAccess) {
            return true;
        }
        // filter imported syms
        if (importDeclCount < importDeclSyms.size() && importDeclSyms.count(sym.id)) {
            importDeclCount++;
            return true;
        }
        // filter not top decl
        if (sym.scope.find(':') != std::string::npos) {
            return true;
        }
        // filter by modifier
        bool isAccessible =
            sym.modifier == Modifier::PUBLIC
            || (relation == PackageRelation::CHILD && (sym.modifier == Modifier::INTERNAL
                                                       || sym.modifier == Modifier::PROTECTED))
            || (relation == PackageRelation::SAME_MODULE && sym.modifier == Modifier::PROTECTED)
            || (relation == PackageRelation::PARENT && sym.modifier == Modifier::PROTECTED);
        // filter root pkg symbols if cur module is combined
        if (isAccessible
            && !CompilerCodiraProject::GetInstance()->IsCombinedSym(curModule, curPkgName, symPackage)) {
            callback(symPackage, sym);
        }
        return true;
    });
}

void BackgroundIndexDB::FindExtendSymsOnCompletion(const SymbolID &dotCompleteSym,
    const std::unordered_set<SymbolID> &visibleMembers,
    const std::string &curPkgName, const std::string &curModule,
    const std::function<void(const std::string &, const std::string &,
        const Symbol &, const CompletionItem &)>& callback)
{
    if (Options::GetInstance().IsOptionSet("test")) {
        return;
    }
    std::unordered_set<std::string> curModuleDeps =
        CompilerCodiraProject::GetInstance()->GetOneModuleDirectDeps(curModule);
    db.GetExtendItem(GetArrayFromID(dotCompleteSym),
        [&](const std::string &packageName, const Symbol &sym,
            const ExtendItem &extendIem, const CompletionItem &completionItem) {
            if (curPkgName == packageName) {
                return;
            }
            if (visibleMembers.find(extendIem.id) != visibleMembers.end()) {
                return;
            }
            auto relation = GetPackageRelation(curPkgName, packageName);
            auto checkAccessible = [&relation](const Modifier& modifier) -> bool {
                bool isAccessible =
                    modifier == Modifier::PUBLIC
                    || (relation == PackageRelation::CHILD && (modifier == Modifier::INTERNAL
                                                            || modifier == Modifier::PROTECTED))
                    || (relation == PackageRelation::SAME_MODULE &&
                        modifier == Modifier::PROTECTED)
                    || (relation == PackageRelation::PARENT && modifier == Modifier::PROTECTED);
                return isAccessible;
            };
            if (!sym.isCodeoSym && !curModuleDeps.count(sym.curModule)) {
                return;
            }
            // filter by modifier
            if (checkAccessible(extendIem.modifier) && checkAccessible(sym.modifier)) {
                callback(packageName, extendIem.interfaceName, sym, completionItem);
            }
    });
}

void BackgroundIndexDB::FindComment(const Symbol &sym, std::vector<std::string> &comments)
{
    db.GetComment(GetArrayFromID(sym.id), [&](const Comment &comment) {
        comments.push_back(comment.commentStr);
        return true;
    });
}

void BackgroundIndexDB::FindCrossSymbolByName(const std::string &packageName, const std::string &symName,
    bool isComebined, const std::function<void(const CrossSymbol &)> &callback)
{
    std::unordered_set<std::string> targetPackageSet;
    targetPackageSet.insert(packageName);
    if (isComebined) {
        const auto pkgMap = CompilerCodiraProject::GetInstance()->GetFullPkgNameToPathMap();
        for (auto &item : pkgMap) {
            std::string pkgName = item.first;
            if (pkgName.find(packageName) != std::string::npos) {
                targetPackageSet.insert(pkgName);
            }
        }
    }
    for (const auto &pkgName : targetPackageSet) {
        db.GetCrossSymbols(pkgName, symName, callback);
    }
}

} // namespace lsp
} // namespace ark

