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

#ifndef LSPSERVER_INDEX_INDEXSTORAGE_H
#define LSPSERVER_INDEX_INDEXSTORAGE_H

#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include "../../../third_party/flatbuffers/include/index_generated.h"
#include "MemIndex.h"

namespace ark {
namespace lsp {

using ASTData = std::vector<uint8_t>;
struct FileIn {
    virtual ~FileIn() = default;
    std::string shardId;
};

struct FileOut {
    virtual ~FileOut() = default;
    std::string shardId;
};

struct AstFileIn : public FileIn {
    ASTData data;
};

struct AstFileOut : public FileOut {
    const ASTData *data = nullptr;
};

class FileHandler {
public:
    virtual ~FileHandler() = default;

    virtual std::optional<std::unique_ptr<FileIn>> LoadShard(std::string filePath) const = 0;

    virtual void StoreShard(std::string fullPkgName, const FileOut *out) const = 0;
};

class AstFileHandler : public FileHandler {
public:
    AstFileHandler() = default;

    std::optional<std::unique_ptr<FileIn>> LoadShard(std::string filePath) const override;

    void StoreShard(std::string filePath, const FileOut *out) const override;
};

class IndexFileHandler : public FileHandler {
    IndexFileHandler() = default;
};

struct IndexFileIn {
    SymbolSlab symbols;
    RefSlab refs;
    RelationSlab relations;
    ExtendSlab extends;
    CrossSymbolSlab crossSymbos;
};

struct IndexFileOut {
    const SymbolSlab *symbols = nullptr;
    const RefSlab *refs = nullptr;
    const RelationSlab *relations = nullptr;
    const ExtendSlab *extends = nullptr;
    const CrossSymbolSlab *crossSymbos = nullptr;

    IndexFileOut() = default;
};

class CacheManager {
public:
    explicit CacheManager(const std::string &workspace = "./") : basePath(workspace)
    {
        std::string cacheRoot = FileUtil::JoinPath(basePath, ".cache/");
        if (!FileUtil::FileExist(cacheRoot)) {
            auto ret = FileUtil::CreateDirs(cacheRoot);
            if (ret == -1) {
                return;
            }
        }
    }

    void InitDir();

    bool IsStale(const std::string &pkgName, const std::string &digest);

    void UpdateIdMap(const std::string &pkgName, const std::string &digest);

    void Store(const std::string &pkgName, const std::string &digest, const std::vector<uint8_t> &buffer);

    std::optional<std::unique_ptr<FileIn>> Load(const std::string &pkgName);

    std::optional<std::unique_ptr<IndexFileIn>> LoadIndexShard(const std::string &curPkgName,
                                                          const std::string &shardIdentifier) const;

    void StoreIndexShard(const std::string &curPkgName, const std::string &shardIdentifier,
                    const IndexFileOut &shard) const;

    std::string GetShardPathFromFilePath(std::string curPkgName,
                                         const std::string &shardIdentifier) const;

    std::unique_ptr<AstFileHandler> astLoader = std::make_unique<AstFileHandler>();

    void readRefs(
        const IdxFormat::HashedPackage &package, std::unique_ptr<ark::lsp::IndexFileIn> &ifi) const;

    void readExtends(
        const IdxFormat::HashedPackage &package, std::unique_ptr<ark::lsp::IndexFileIn> &ifi) const;

    void readCrossSymbols(
        const IdxFormat::HashedPackage &package, std::unique_ptr<ark::lsp::IndexFileIn> &ifi) const;
private:
    std::string basePath;
    std::string astdataDir;
    std::string indexDir;
    std::mutex cacheMtx;
    std::unordered_map<std::string, std::string> astIdMap;
};
} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_INDEXSTORAGE_H
