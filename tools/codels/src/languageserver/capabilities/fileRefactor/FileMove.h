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


#ifndef CODIRA_LSP_FILEMOVE_H
#define CODIRA_LSP_FILEMOVE_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../CompilerCodiraProject.h"
#include "../../index/Symbol.h"
#include "../../common/Utils.h"
#include "FileRefactor.h"

namespace ark {
class FileMove {
public:
    static void FileMoveRefactor(const ArkAST *ast, FileRefactorRespParams &result, const std::string &file, const std::string &selectedElement, const std::string &target);

private:
    static void FindFileRefactor(const ArkAST *ast, const std::string &file, const std::string &targetPkg, const std::string &targetPath, FileRefactorRespParams &result);

    static void DealMoveFile(const ArkAST *ast, const std::string &file, const std::string &targetPkg, const std::string &targetPath, FileRefactor &refactor);

    static void DealRefFile(const ArkAST *ast, const std::string &file, const std::string &targetPkg, FileRefactor &refactor);

    static void DealReExport(const ArkAST *ast, const std::string &file, const std::string &targetPkg, FileRefactor &refactor);

    static void DealMoveFilePackageName(const ArkAST *ast, const std::string &targetPkg, const std::string &targetPath, FileRefactorRespParams &result);

    static std::string GetFullPkgBySymScope(const std::string &symScope);

    static ArkAST* GetRefArkAST(const std::string &filePath);

    static std::unique_ptr<PackageInstance> GetPackageInstance(LSPCompilerInstance *ci);

    static std::unique_ptr<ArkAST> CreateArkAST(LSPCompilerInstance *ci, PackageInstance *pkgInstance, const std::string &filePath);

    static void Clear();

    static std::string GetRealImportSymName(const lsp::Symbol &sym);

    static bool IsValidExportSym(const lsp::Symbol &sym, const std::string &exportedPkg);

    static bool IsValidExportSym(const lsp::Symbol &sym, const std::string &exportedPkg, const std::string &fullPkgSym);

    static std::string GetPkgNameAfterMove(const std::string &pathBeforeMove, std::string pkgBeforeMove);

    static File* GetFileNode(const ArkAST *ast, std::string filePath);

    static bool ExistImportForTargetPkg(lsp::SymbolID symbolID, std::string targetPkg, std::string moveFile);

    static std::string GetTargetPath(std::string file);

    static bool isInvalidImport(Ptr<ImportSpec> fileImport);

    static std::unordered_map<std::string, std::unique_ptr<Codira::LSPCompilerInstance>> ciMap;

    static std::unordered_map<std::string, std::unique_ptr<ArkAST>> astMap;

    static std::unordered_map<std::string, std::unique_ptr<PackageInstance>> pkgInstanceMap;

    // if moving a folder, store the move dir
    static std::string moveDirPath;

    static std::string targetDir;
};
}


#endif // CODIRA_LSP_FILEMOVE_H
