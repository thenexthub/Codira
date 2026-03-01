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

#ifndef LSPSERVER_INDEX_BACKGROUNDDB_H
#define LSPSERVER_INDEX_BACKGROUNDDB_H

#include <mutex>
#include "IndexDatabase.h"
#include "IndexStorage.h"
#include "MemIndex.h"

namespace ark {
namespace lsp {

class BackgroundIndexDB final: public SymbolIndex {
public:
    /**
     * Creates a new background index and starts its threads.
     * The current Context will be propagated to each worker thread.
     */
    explicit BackgroundIndexDB(IndexDatabase &indexDB);

    ~BackgroundIndexDB() override; // Blocks while the current task finishes.

    void FuzzyFind(const FuzzyFindRequest &req,
        std::function<void(const Symbol &)> callback) const override;

    void Lookup(const LookupRequest &req,
        std::function<void(const Symbol &)> callback) const override;

    void GetExportSID(IDArray array, std::function<void(const CrossSymbol &)> callback) const override;

    void FindPkgSyms(const PkgSymsRequest &req, std::function<void(const Symbol &)> callback) const override;

    void Refs(const RefsRequest &req,
        std::function<void(const Ref &)> callback) const override;

    void RefsFindReference(const RefsRequest &req, Ref &definition,
        std::function<void(const Ref &)> callback) const override;

    void FileRefs(const FileRefsRequest &req,
        std::function<void(const Ref &ref, const SymbolID symId)> callback) const override;

    void Relations(const RelationsRequest &req,
        std::function<void(const Relation &)>callback) const override;

    void Callees(const std::string &pkgName, const SymbolID &declId,
                 std::function<void(const SymbolID &, const Ref &)> callback) const override;

    /**
     * Given index results from a TU, only update symbols coming from files with
     * different digests than ShardVersionsSnapshot. Also stores new index
     * information on IndexStorage.
     */
    void Update(const std::string &curPkgName, IndexFileOut index);
    void UpdateAll(const std::map<int, std::vector<std::string>> &fileMap, std::unique_ptr<lsp::MemIndex> idxs);
    void UpdateFile(const std::map<int, std::vector<std::string>> &fileMap);
    void DeleteFiles(const std::unordered_set<std::string> &fileSet);
    std::string GetFileDigest(uint32_t fileId);

    void FindRiddenUp(SymbolID id, std::unordered_set<SymbolID> &ids, SymbolID &topId) override
    {
        std::unordered_set<SymbolID> newAdds;
        db.GetRelationsRiddenUp(id, RelationKind::RIDDEND_BY, [&](const Relation &rel) {
            if (ids.emplace(rel.subject).second) {
                newAdds.insert(rel.subject);
            }
            topId = rel.subject;
            return true;
        });
        for (auto item: newAdds) {
            FindRiddenUp(item, ids, topId);
        }
    }

    void FindRiddenDown(SymbolID id, std::unordered_set<SymbolID> &ids) override
    {
        std::unordered_set<SymbolID> newAdds;
        db.GetRelationsRiddenDown(id, RelationKind::RIDDEND_BY, [&](const Relation &rel) {
            if (ids.emplace(rel.object).second) {
                newAdds.insert(rel.object);
            }
            return true;
        });
        for (auto item: newAdds) {
            FindRiddenDown(item, ids);
        }
    }

    IndexDatabase& GetIndexDatabase() const
    {
        return db;
    }

    void GetFuncByName(const FuncSigRequest &req,
                       std::function<void(const Symbol &)> callback) const override;
					   
    Symbol GetAimSymbol(const Decl& decl) override;

    void FindImportSymsOnCompletion(
        const std::pair<std::unordered_set<SymbolID>, std::unordered_set<SymbolID>>& filterSyms,
        const std::string &curPkgName, const std::string &curModule, const std::string &prefix,
        std::function<void(const std::string &, const Symbol &, const CompletionItem &)> callback) override;

    void FindImportSymsOnQuickFix(const std::string &curPkgName, const std::string &curModule,
        const std::unordered_set<SymbolID> &importDeclSyms,
        const std::string& identifier,
        const std::function<void(const std::string &, const Symbol &)>& callback) override;

    void FindExtendSymsOnCompletion(const SymbolID &dotCompleteSym,
        const std::unordered_set<SymbolID> &visibleMembers,
        const std::string &curPkgName, const std::string &curModule,
        const std::function<void(const std::string &, const std::string &,
            const Symbol &, const CompletionItem &)>& callback) override;

    void FindComment(const Symbol &sym, std::vector<std::string> &comments) override;

    void FindCrossSymbolByName(const std::string &packageName, const std::string &symName, bool isComebined,
        const std::function<void(const CrossSymbol &)> &callback) override;;

private:
    // Database
    IndexDatabase &db;
};

} // namespace lsp
} // namespace ark

#endif // LSPSERVER_INDEX_BACKGROUNDDB_H
