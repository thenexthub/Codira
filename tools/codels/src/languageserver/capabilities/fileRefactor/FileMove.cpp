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

#include "FileMove.h"

#include <utility>

namespace ark {
std::unordered_map<std::string, std::unique_ptr<Codira::LSPCompilerInstance>> FileMove::ciMap = {};
std::unordered_map<std::string, std::unique_ptr<ArkAST>> FileMove::astMap = {};
std::unordered_map<std::string, std::unique_ptr<PackageInstance>> FileMove::pkgInstanceMap = {};
std::string FileMove::moveDirPath;
std::string FileMove::targetDir;

void FileMove::FileMoveRefactor(const ArkAST *ast,
    FileRefactorRespParams &result,
    const std::string &file,
    const std::string &selectedElement,
    const std::string &target)
{
    if (!ast || !ast->file || !ast->file->curPackage) {
        Logger::Instance().LogMessage(MessageType::MSG_LOG, "FileRefactor: Get ArkAst fail for move file: " + file);
        return;
    }
    bool isDir = FileUtil::GetFileName(selectedElement).find('.') == std::string::npos;

    // not support cross module moving file
    std::string pkg;
    if (isDir) {
        pkg = CompilerCodiraProject::GetInstance()->GetFullPkgByDir(selectedElement);
        if (pkg.empty()) {
            Logger::Instance().LogMessage(
                MessageType::MSG_LOG, "FileRefactor: Cannot find fullPackageName for move file: " + selectedElement);
            return;
        }
    } else {
        pkg = ast->file->curPackage->fullPackageName;
    }
    std::string targetFullPkg = CompilerCodiraProject::GetInstance()->GetFullPkgByDir(target);
    if (targetFullPkg.empty()) {
        Logger::Instance().LogMessage(
            MessageType::MSG_LOG, "FileRefactor: Cannot find fullPackageName for target pkg: " + target);
        return;
    }
    PackageRelation relation = FileRefactor::GetPackageRelation(pkg, targetFullPkg);
    if (relation == PackageRelation::DIFF_MODULE) {
        return;
    }

    // not support moving root package
    if (isDir && pkg.find('.') == std::string::npos) {
        return;
    }
    FileMove::Clear();

    targetDir = target;
    if (isDir) {
        std::string tmpPkg = pkg;
        size_t idx = pkg.find_last_of('.');
        if (idx != std::string::npos) {
            tmpPkg = pkg.substr(0, idx);
        }

        std::string relativePath = file.substr(FileUtil::GetDirPath(selectedElement).length());
        std::string targetPath = FileUtil::JoinPath(target, relativePath);
        std::string fPkg = ast->file->curPackage->fullPackageName;
        std::string subPkg;
        if (fPkg.length() > tmpPkg.length()) {
            subPkg = fPkg.substr(tmpPkg.length());
        }
        std::string realTargetPkg = targetFullPkg + subPkg;
        moveDirPath = selectedElement;
        FindFileRefactor(ast, file, realTargetPkg, targetPath, result);
    } else {
        std::string targetPath = FileUtil::JoinPath(target, FileUtil::GetFileName(file));
        FindFileRefactor(ast, file, targetFullPkg, targetPath, result);
    }
}
void FileMove::FindFileRefactor(const ArkAST *ast, const std::string &file, const std::string &targetPkg,
    const std::string &targetPath, FileRefactorRespParams &result)
{
    FileRefactor refactor = FileRefactor(result);
    refactor.moveDir = !moveDirPath.empty();
    // 1.change package name for moving file
    FileMove::DealMoveFilePackageName(ast, targetPkg, targetPath, result);
    // 2.change import for moving file
    FileMove::DealMoveFile(ast, file, targetPkg, targetPath, refactor);
    // 3.change import for files that used moving file decls
    FileMove::DealRefFile(ast, file, targetPkg, refactor);
    // 4.change import for files that used moving file re-export decls
    FileMove::DealReExport(ast, file, targetPkg, refactor);
}

void FileMove::DealMoveFile(const ArkAST *ast, const std::string &file,
    const std::string &targetPkg, const std::string &targetPath, FileRefactor &refactor)
{
    unsigned int fileId = ast->fileID;
    std::string fullPkgName = ast->file->curPackage->fullPackageName;
    auto index = ark::CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }
    auto fileNode = FileMove::GetFileNode(ast, file);
    if (!fileNode) {
        return;
    }
    refactor.fileNode = fileNode;
    refactor.file = file;
    lsp::FileRefsRequest fileRefReq{fileId, file, fullPkgName, lsp::RefKind::REFERENCE};
    std::unordered_set<lsp::SymbolID> fileRefIds;
    index->FileRefs(
        fileRefReq, [&fileRefIds, &fileId](const lsp::Ref &ref, const lsp::SymbolID symId) {
            if (ref.location.IsZeroLoc()) {
                return;
            }
            fileRefIds.insert(symId);
        });

    lsp::LookupRequest lookupReq{fileRefIds};
    index->Lookup(lookupReq, [&refactor, &targetPkg, &targetPath, &file](const lsp::Symbol &sym) {
        if (sym.location.fileUri == file || sym.location.fileUri.empty()) {
            return;
        }
        std::string pkg = FileMove::GetFullPkgBySymScope(sym.scope);
        if (!FileMove::IsValidExportSym(sym, pkg) || pkg.empty()) {
            return;
        }
        pkg = FileMove::GetPkgNameAfterMove(sym.location.fileUri, pkg);
        PackageRelation relation = FileRefactor::GetPackageRelation(targetPkg, pkg);
        if (relation == PackageRelation::SAME_PACKAGE) {
            return;
        }
        refactor.accessForTargetPkg = FileMove::ExistImportForTargetPkg(sym.id, targetPkg, file);
        refactor.refactorPkg = pkg;
        refactor.newPkg = pkg;
        refactor.sym = FileMove::GetRealImportSymName(sym);
        refactor.kind = FileRefactorKind::RefactorMoveFile;
        refactor.reExportedPkg = "";
        refactor.targetPath = targetPath;
        refactor.MatchRefactor(FileRefactorKind::RefactorMoveFile, relation, sym.modifier);
    });

    refactor.accessForTargetPkg = false;
    refactor.refactorPkg = targetPkg;
    refactor.newPkg = "";
    refactor.sym = "";
    refactor.kind = FileRefactorKind::RefactorMoveFile;
    refactor.reExportedPkg = "";
    refactor.targetPath = targetPath;
    refactor.MatchRefactor(FileRefactorKind::RefactorMoveFile,
        PackageRelation::SAME_PACKAGE, lsp::Modifier::UNDEFINED);
}

void FileMove::DealRefFile(const ArkAST *ast, const std::string &file, const std::string &targetPkg,
    FileRefactor &refactor)
{
    unsigned int fileId = ast->fileID;
    std::string fullPkgName = ast->file->curPackage->fullPackageName;
    auto index = ark::CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }
    lsp::FileRefsRequest fileDefReq{fileId, file, fullPkgName, lsp::RefKind::DEFINITION};
    std::unordered_set<lsp::SymbolID> fileDefIds;
    index->FileRefs(fileDefReq, [&fileDefIds](const lsp::Ref &ref, const lsp::SymbolID symId) {
        fileDefIds.insert(symId);
    });

    auto DealSingleRef = [&file, &targetPkg, &refactor, &fullPkgName]
        (std::string &symName, ark::lsp::Modifier modifier, const lsp::Ref &ref) {
            if (ref.location.IsZeroLoc() || ref.location.fileUri == file) {
                return;
            }
            std::string refFullPkg = CompilerCodiraProject::GetInstance()->GetFullPkgName(ref.location.fileUri);
            if (FileRefactor::GetPackageRelation(fullPkgName, refFullPkg) == PackageRelation::DIFF_MODULE) {
                return;
            }
            ArkAST *refAst = FileMove::GetRefArkAST(ref.location.fileUri);

            if (!refAst || !refAst->file) {
                Logger::Instance().LogMessage(MessageType::MSG_LOG,
                    "DealRefFile: Cannot find ArkAst for the file: " + ref.location.fileUri);
                return;
            }
            auto fileNode = FileMove::GetFileNode(refAst, ref.location.fileUri);
            if (!fileNode) {
                return;
            }
            refactor.accessForTargetPkg = false;
            refactor.fileNode = fileNode;
            refactor.file = ref.location.fileUri;
            refactor.sym = symName;
            refactor.refactorPkg = fullPkgName;
            refactor.newPkg = targetPkg;
            refactor.kind = FileRefactorKind::RefactorRefFile;
            refactor.reExportedPkg = "";
            refactor.targetPath = FileMove::GetTargetPath(ref.location.fileUri);
            PackageRelation relation = FileRefactor::GetPackageRelation(
                FileMove::GetPkgNameAfterMove(ref.location.fileUri, refFullPkg), targetPkg);
            refactor.MatchRefactor(FileRefactorKind::RefactorRefFile, relation, modifier);
    };

    std::for_each(fileDefIds.begin(), fileDefIds.end(),
        [&index, &DealSingleRef, &fullPkgName](const lsp::SymbolID symId) {
        std::string symName;
        ark::lsp::Modifier modifier;
        lsp::LookupRequest lookupReq{{symId}};
        index->Lookup(lookupReq, [&symName, &modifier, &fullPkgName](const lsp::Symbol &sym) {
            if (!FileMove::IsValidExportSym(sym, fullPkgName)) {
                symName = "";
                return;
            }
            symName = FileMove::GetRealImportSymName(sym);
            modifier = sym.modifier;
        });
        if (symName.empty()) {
            return;
        }
        std::unordered_set<std::string> processedFiles;
        const lsp::RefsRequest refReq{{symId}, lsp::RefKind::REFERENCE};
        index->Refs(refReq, [&symName, &modifier, &processedFiles, &DealSingleRef](const lsp::Ref &ref) {
            if (processedFiles.count(ref.location.fileUri)) {
                return;
            }
            DealSingleRef(symName, modifier, ref);
            processedFiles.insert(ref.location.fileUri);
        });

        const lsp::RefsRequest importRefReq{{symId}, lsp::RefKind::IMPORT};
        index->Refs(importRefReq, [&symName, &modifier, &processedFiles, &DealSingleRef](const lsp::Ref &ref) {
            if (ref.location.IsZeroLoc()) {
                return;
            }
            if (processedFiles.count(ref.location.fileUri)) {
                return;
            }
            DealSingleRef(symName, modifier, ref);
            processedFiles.insert(ref.location.fileUri);
        });
    });
}

void FileMove::DealReExport(const ArkAST *ast, const std::string &file, const std::string &targetPkg,
    FileRefactor &refactor)
{
    unsigned int fileId = ast->fileID;
    std::string fullPkgName = ast->file->curPackage->fullPackageName;
    auto index = ark::CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index || !ast->packageInstance) {
        return;
    }
    auto DealSingleReExportRef = [&file, &refactor, &fullPkgName, &targetPkg]
        (std::string symName, lsp::Modifier modifier, std::string originPkg, const lsp::Ref &ref) {
            if (ref.location.IsZeroLoc() || ref.location.fileUri == file) {
                return;
            }
            std::string refFullPkg = CompilerCodiraProject::GetInstance()->GetFullPkgName(ref.location.fileUri);
            if (refFullPkg == originPkg || FileRefactor::GetPackageRelation(fullPkgName, refFullPkg)
                                               == PackageRelation::DIFF_MODULE) {
                return;
            }
            ArkAST *refAst = FileMove::GetRefArkAST(ref.location.fileUri);
            if (!refAst || !refAst->file || !refAst->file->curPackage) {
                Logger::Instance().LogMessage(MessageType::MSG_LOG,
                    "DealReExport: Cannot find ArkAst for the file: " + ref.location.fileUri);
                return;
            }
            auto fileNode = FileMove::GetFileNode(refAst, ref.location.fileUri);
            if (!fileNode) {
                return;
            }
            refactor.accessForTargetPkg = false;
            refactor.fileNode = fileNode;
            refactor.file = ref.location.fileUri;
            refactor.sym = std::move(symName);
            refactor.refactorPkg = fullPkgName;
            refactor.newPkg = targetPkg;
            refactor.reExportedPkg = std::move(originPkg);
            refactor.kind = FileRefactorKind::RefactorReExport;
            refactor.targetPath = FileMove::GetTargetPath(ref.location.fileUri);
            PackageRelation relation = FileRefactor::GetPackageRelation(
                FileMove::GetPkgNameAfterMove(ref.location.fileUri, refFullPkg), targetPkg);
            refactor.MatchRefactor(FileRefactorKind::RefactorReExport, relation, modifier);
    };

    auto DealSingleReExport = [&index, &DealSingleReExportRef](std::vector<lsp::Symbol> &reExportSyms,
                                  lsp::Modifier modifier, std::string originPkg) {
        for (const auto &sym : reExportSyms) {
            std::string symName = FileMove::GetRealImportSymName(sym);
            std::unordered_set<std::string> processedFiles;
            const lsp::RefsRequest refReq{{sym.id}, lsp::RefKind::REFERENCE};
            index->Refs(refReq, [&symName, &modifier, &originPkg, &processedFiles, &DealSingleReExportRef]
                (const lsp::Ref &ref) {
                if (processedFiles.count(ref.location.fileUri)) {
                    return;
                }
                DealSingleReExportRef(symName, modifier, originPkg, ref);
                processedFiles.insert(ref.location.fileUri);
            });

            const lsp::RefsRequest importRefReq{{sym.id}, lsp::RefKind::IMPORT};
            index->Refs(importRefReq, [&symName, &modifier, &originPkg, &processedFiles, &DealSingleReExportRef]
                (const lsp::Ref &ref) {
                if (processedFiles.count(ref.location.fileUri)) {
                    return;
                }
                DealSingleReExportRef(symName, modifier, originPkg, ref);
                processedFiles.insert(ref.location.fileUri);
            });
        }
    };

    for (auto &fileImport : ast->file->imports) {
        if (FileMove::isInvalidImport(fileImport.get())) {
            continue;
        }
        auto importContent = fileImport.get()->content;
        lsp::Modifier modifier = FileRefactor::GetImportModifier(*fileImport);
        std::string importFullPkg = FileRefactor::GetImportFullPkg(fileImport->content);
        const lsp::PkgSymsRequest pkgSymsRequest = {importFullPkg};
        std::vector<lsp::Symbol> reExportSyms;
        if (importContent.kind == ImportKind::IMPORT_ALL) {
            index->FindPkgSyms(pkgSymsRequest, [&reExportSyms, &importFullPkg](const lsp::Symbol &sym) {
                if (!FileMove::IsValidExportSym(sym, importFullPkg)) {
                    return;
                }
                reExportSyms.emplace_back(sym);
            });
            DealSingleReExport(reExportSyms, modifier, importFullPkg);
            continue;
        }
        std::string fullImportSym = FileRefactor::GetImportFullSymWithoutAlias(fileImport->content);
        index->FindPkgSyms(pkgSymsRequest, [&reExportSyms, &fullImportSym, &importFullPkg]
            (const lsp::Symbol &sym) {
                if (FileMove::IsValidExportSym(sym, importFullPkg, fullImportSym)) {
                    reExportSyms.emplace_back(sym);
                }
            });
        DealSingleReExport(reExportSyms, modifier, importFullPkg);
    }
}

void FileMove::DealMoveFilePackageName(const ArkAST *ast, const std::string &targetPkg,
    const std::string &targetPath, FileRefactorRespParams &result)
{
    if (!ast->file || !ast->file->package) {
        return;
    }
    std::string uri = URI::URIFromAbsolutePath(targetPath).ToString();
    Position fullPackageNameBegin = ast->file->package->prefixPoses.empty() ? ast->file->package->packageName.Begin()
                                                                            : ast->file->package->prefixPoses[0];
    Range changeRange = {fullPackageNameBegin, ast->file->package->packageName.End()};
    changeRange = TransformFromChar2IDE(changeRange);
    result.changes[uri].insert({FileRefactorChangeType::CHANGED, changeRange, targetPkg});
}

std::string FileMove::GetFullPkgBySymScope(const std::string &symScope)
{
    size_t colonPos = symScope.find(':');
    return colonPos == std::string::npos ? symScope : symScope.substr(0, colonPos);
}

ArkAST* FileMove::GetRefArkAST(const std::string &filePath)
{
    std::string refFullPkg = CompilerCodiraProject::GetInstance()->GetFullPkgName(filePath);
    if (astMap.count(refFullPkg)) {
        return astMap[refFullPkg].get();
    }
    ArkAST *refAst = CompilerCodiraProject::GetInstance()->GetArkAST(filePath);
    std::unique_ptr<Codira::LSPCompilerInstance> ci;
    std::unique_ptr<ArkAST> ast;
    std::unique_ptr<PackageInstance> pkgInstance;
    if (!CompilerCodiraProject::GetInstance()->pLRUCache->HasCache(refFullPkg) || !refAst) {
        ci = CompilerCodiraProject::GetInstance()->GetCIForFileRefactor(filePath);
        if (!ci) {
            return nullptr;
        }
        pkgInstance = GetPackageInstance(ci.get());
        if (!pkgInstance) {
            return nullptr;
        }
        ast = FileMove::CreateArkAST(ci.get(), pkgInstance.get(), filePath);
        if (!ast) {
            return nullptr;
        }
        ciMap[refFullPkg] = std::move(ci);
        pkgInstanceMap[refFullPkg] = std::move(pkgInstance);
        astMap[refFullPkg] = std::move(ast);
        return astMap[refFullPkg].get();
    }
    return refAst;
}
std::unique_ptr<PackageInstance> FileMove::GetPackageInstance(LSPCompilerInstance *ci)
{
    if (ci->GetSourcePackages().empty()) {
        return nullptr;
    }
    auto pkg = ci->GetSourcePackages().front();
    if (!pkg) {
        return nullptr;
    }
    auto pkgInstance = std::make_unique<PackageInstance>(ci->diag, ci->importManager);
    if (!pkgInstance) {
        return nullptr;
    }
    pkgInstance->package = pkg;
    pkgInstance->ctx = nullptr;
    return pkgInstance;
}

std::unique_ptr<ArkAST> FileMove::CreateArkAST(LSPCompilerInstance *ci,
    PackageInstance *pkgInstance, const std::string &filePath)
{
    if (ci->GetSourcePackages().empty()) {
        return nullptr;
    }
    auto pkg = ci->GetSourcePackages().front();
    if (!pkg) {
        return nullptr;
    }

    for (auto &f : pkg->files) {
        if (!f || f->filePath != filePath) {
            continue;
        }
        std::pair<std::string, std::string> paths = { f->filePath, ci->bufferCache[filePath] };
        auto arkAST = std::make_unique<ArkAST>(paths, f.get(), ci->diag,
            pkgInstance, &ci->GetSourceManager());
        std::string absName = FileStore::NormalizePath(f->filePath);
        int fileId = ci->GetSourceManager().GetFileID(absName);
        if (fileId >= 0) {
            arkAST->fileID = static_cast<unsigned int>(fileId);
        }
        return arkAST;
    }
    return nullptr;
}
void FileMove::Clear()
{
    ciMap.clear();
    pkgInstanceMap.clear();
    astMap.clear();
    moveDirPath = "";
    targetDir = "";
}

std::string FileMove::GetRealImportSymName(const lsp::Symbol &sym)
{
    if (sym.name != "init") {
        return sym.name;
    }
    auto it = sym.scope.find_last_of(':');
    if (it == std::string::npos) {
        return sym.name;
    }
    return sym.scope.substr(it + 1);
}

bool FileMove::IsValidExportSym(const lsp::Symbol &sym, const std::string &exportedPkg)
{
    if (sym.location.end.IsZero() && sym.name != "init") {
        return false;
    }
    if (sym.scope == exportedPkg) {
        return true;
    }
    std::string constructorPrefix = "init(";
    if (sym.kind == ASTKind::FUNC_DECL && sym.signature.length() > constructorPrefix.length()
        && sym.signature.substr(0, constructorPrefix.length()) == constructorPrefix){
        return true;
    }
    return false;
}

bool FileMove::IsValidExportSym(const lsp::Symbol &sym, const std::string &exportedPkg, const std::string &fullPkgSym)
{
    if (sym.location.end.IsZero() && sym.name != "init") {
        return false;
    }
    if (fullPkgSym == exportedPkg + CONSTANTS::DOT + sym.name) {
        return true;
    }
    auto it = fullPkgSym.find_last_of('.');
    if (it == std::string::npos) {
        return false;
    }
    std::string importSym = fullPkgSym.substr(it + 1);
    std::string qualifierScope = exportedPkg + CONSTANTS::COMMA + importSym;
    if (sym.scope != qualifierScope) {
        return false;
    }
    std::string constructorPrefix = "init(";
    if (sym.kind == ASTKind::FUNC_DECL && sym.signature.length() > constructorPrefix.length()
        && sym.signature.substr(0, constructorPrefix.length()) == constructorPrefix){
        return true;
        }
    return false;
}

std::string FileMove::GetPkgNameAfterMove(const std::string &pathBeforeMove, std::string pkgBeforeMove)
{
    if (moveDirPath.empty()) {
        return pkgBeforeMove;
    }
    if (!IsUnderPath(moveDirPath, pathBeforeMove)) {
        return pkgBeforeMove;
    }
    std::string targetPkg = CompilerCodiraProject::GetInstance()->GetFullPkgByDir(targetDir);
    std::string moveDirPkg = CompilerCodiraProject::GetInstance()->GetFullPkgByDir(moveDirPath);
    std::string tmpPkg = moveDirPkg;
    size_t idx = moveDirPkg.find_last_of('.');
    if (idx != std::string::npos) {
        tmpPkg = moveDirPkg.substr(0, idx);
    }
    std::string subPkg;
    if (pkgBeforeMove.length() > tmpPkg.length()) {
        subPkg = pkgBeforeMove.substr(tmpPkg.length());
    }
    std::string pkgNameAfterMove = targetPkg + subPkg;
    return pkgNameAfterMove;
}

std::string FileMove::GetTargetPath(std::string file)
{
    if (moveDirPath.empty()) {
        return file;
    }
    if (!IsUnderPath(moveDirPath, file)) {
        return file;
    }
    std::string relativePath = file.substr(FileUtil::GetDirPath(moveDirPath).length());
    std::string targetPath = FileUtil::JoinPath(targetDir, relativePath);
    return targetPath;
}

bool FileMove::isInvalidImport(Ptr<ImportSpec> fileImport)
{
    if (!fileImport || fileImport->end.IsZero() || !fileImport->modifier) {
        return true;
    }
    auto importContent = fileImport.get()->content;
    if (importContent.kind == ImportKind::IMPORT_MULTI) {
        return true;
    }
    lsp::Modifier modifier = FileRefactor::GetImportModifier(*fileImport);
    if (!FileRefactor::ValidReExportModifier(modifier)) {
        return true;
    }
    return false;
}

File* FileMove::GetFileNode(const ArkAST *ast, std::string filePath)
{
    if (!ast->file || !ast->file->curPackage || ast->file->curPackage->files.empty()) {
        return nullptr;
    }
    for (const auto &f : ast->file->curPackage->files) {
        if(f->filePath == filePath) {
            return f.get();
        }
    }
    return nullptr;
}

bool FileMove::ExistImportForTargetPkg(lsp::SymbolID symbolID, std::string targetPkg, std::string moveFile)
{
    auto index = ark::CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return false;
    }
    const lsp::RefsRequest importRefReq{{symbolID}, lsp::RefKind::IMPORT};
    bool isImport = false;
    auto DealImport = [](File *f, Position start, Position end, bool isMoveFile) {
        for (const auto &importSpec : f->imports) {
            bool isContainImport = isMoveFile && importSpec && importSpec->begin <= start && importSpec->end >= end;
            if (isContainImport) {
                return true;
            }
            bool isReexport = importSpec && importSpec->modifier &&
                              FileRefactor::ReExportKinds.count(importSpec->modifier->modifier) &&
                              importSpec->begin <= start && importSpec->end >= end;
            if (isReexport) {
                return true;
            }
        }
        return false;
    };
    index->Refs(importRefReq, [&targetPkg, &isImport, &DealImport, &moveFile](const lsp::Ref &ref) {
        if (ref.location.IsZeroLoc()) {
            return;
        }
        std::string importFullPkg = CompilerCodiraProject::GetInstance()->GetFullPkgName(ref.location.fileUri);
        PackageRelation relation = FileRefactor::GetPackageRelation(targetPkg, importFullPkg);
        if (relation != PackageRelation::SAME_PACKAGE && ref.location.fileUri != moveFile) {
            return;
        }
        ArkAST *importAst = FileMove::GetRefArkAST(ref.location.fileUri);
        if (!importAst || !importAst->file || !importAst->file->curPackage) {
            Logger::Instance().LogMessage(MessageType::MSG_LOG,
                "DealRefFile: Cannot find ArkAst for the file: " + ref.location.fileUri);
            return;
        }
        for (const auto &f : importAst->file->curPackage->files) {
            if (f->filePath != ref.location.fileUri) {
                continue;
            }
            if(DealImport(f.get(), ref.location.begin, ref.location.end, f->filePath == moveFile)) {
                isImport = true;
                return;
            }
        }
    });
    return isImport;
}

} // namespace ark
