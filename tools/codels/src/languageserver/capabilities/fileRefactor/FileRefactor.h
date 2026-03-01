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

#ifndef CODIRA_LSP_FILEREFACTOR_H
#define CODIRA_LSP_FILEREFACTOR_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../CompilerCodiraProject.h"
#include "../../common/Utils.h"
#include "../../index/Symbol.h"

namespace ark {
enum class FileRefactorKind: uint8_t {
    // refactor move file
    RefactorMoveFile,
    // refactor other file that used symbol defined in the move file
    RefactorRefFile,
    // refactor other file that used symbol re-export in the move file
    RefactorReExport
};

enum class PackageRelation: uint8_t {
    CHILD,
    PARENT,
    SAME_PACKAGE,
    SAME_MODULE,
    DIFF_MODULE
};

class FileRefactor {
public:
    explicit FileRefactor(FileRefactorRespParams &result);
    
    static PackageRelation GetPackageRelation(const std::string &fullPkgName,
        const std::string &targetFullPkgName);
    
    static lsp::Modifier GetImportModifier(ImportSpec &importSpec);

    static bool ValidReExportModifier(lsp::Modifier modifier);

    void MatchRefactor(FileRefactorKind fileRefactorKind, PackageRelation packageRelation,
        ark::lsp::Modifier modifier);
    
    static std::string GetImportFullPkg(const ImportContent &importContent);

    static std::string GetImportFullSym(const ImportContent &importContent);

    static std::string GetImportFullSymWithoutAlias(const ImportContent &importContent);

    static std::unordered_set<TokenKind> ReExportKinds;

    FileRefactorRespParams &result;

    // the fileNode that need add/delete/change import
    const File* fileNode = nullptr;

    // the file that need add/delete/change import
    std::string file;

    // original package name
    std::string refactorPkg;

    // change to package name
    std::string newPkg;

    // reExported package name
    std::string reExportedPkg;

    std::string sym;

    FileRefactorKind kind;

    // the path after the file be moved
    std::string targetPath;

    bool moveDir;

    bool accessForTargetPkg = false;

private:
    void AddImport();

    void DeleteImport();

    void ChangeImport();

    void CheckAndAddImport();

    void CheckAndDeleteImport();

    void CheckAndChangeImport();

    void CheckAndChangeImportForRe();

    void CheckAndDeleteImportForRe();

    void InitMatcher() const;

    bool GetDeletePosInMultiImport(std::vector<Ptr<ImportSpec>> &multiImports,
        ImportContent &singleImport, Position &beginPos, Position &endPos);

    void RemoveDuplicateDelete(Position start, Position end, std::string &uri);

    [[nodiscard]] uint8_t GetRefactorCode(FileRefactorKind fileRefactorKind, PackageRelation packageRelation,
        ark::lsp::Modifier modifier) const;
    
    bool ContainFullSymImport();

    bool FileContainFullSymImport(const File *pFile, bool isRefFile, std::string refactorFullSym);

    bool ContainFullSymImportForReExport();

    bool FileContainFullSymImportForReExport(const File *pFile, bool isRefFile,
        std::string refactorFullSym, std::string originFullSym);

    bool ContainFullPkgImport();

    void CollectImports(std::vector<Ptr<ImportSpec>> &multiImports, std::vector<Ptr<ImportSpec>> &deleteMultiImports,
        const std::string &uri);

    void DeleteMoveFileSingleImport(ImportContent& importContent, Ptr<ImportSpec> fileImport,
        std::vector<Ptr<ImportSpec>> &multiImports, std::vector<Ptr<ImportSpec>> &deleteMultiImports,
        const std::string &uri);

    void DeleteRefFileSingleImport(ImportContent& importContent, Ptr<ImportSpec> fileImport,
        std::vector<Ptr<ImportSpec>> &multiImports, const std::string &uri);

    void DeleteReExportSingleImport(ImportContent& importContent, Ptr<ImportSpec> fileImport,
        std::vector<Ptr<ImportSpec>> &multiImports, const std::string &uri);
};

using RefactorFunc = void (ark::FileRefactor::*)();
class RefactorMatcher {
public:
    static RefactorMatcher &GetInstance()
    {
        static RefactorMatcher instance {};
        return instance;
    }

    void RegFunc(uint8_t kind, const RefactorFunc &func)
    {
        container[kind] = func;
    }

    RefactorFunc GetFunc(uint8_t kind)
    {
        auto item = container.find(kind);
        if (item == container.end()) { return nullptr; }
        return item->second;
    }

private:
    RefactorMatcher() = default;

    ~RefactorMatcher() = default;

    std::map<uint8_t, RefactorFunc> container{};
};
} // namespace ark

#endif // CODIRA_LSP_FILEREFACTOR_H
