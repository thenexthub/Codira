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
 *
 * This file declares the AST Serialization related classes, which provides AST serialization capabilities.
 */

#ifndef CODIRA_MODULES_ASTSERIALIZATION_H
#define CODIRA_MODULES_ASTSERIALIZATION_H

#include <cstdint>
#include <flatbuffers/flatbuffers.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "flatbuffers/ModuleFormat_generated.h"

#include "Codira/AST/Node.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Modules/ASTSerializationTypeDef.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {
constexpr FormattedIndex INVALID_FORMAT_INDEX = 0;
constexpr size_t INITIAL_FILE_SIZE = 65536;
constexpr int FB_MAX_DEPTH = 128;
constexpr int FB_MAX_TABLES = 2000000;
class CodeoManager;

// There are two kinds of decls to be saved based on where the decl is from, we
// use fullId which is defined ad [pkgId, declId] to index decl:
// 1. decl declared in current package, so index it with [currentPkgId, declId]
// 2. decl is imported from other packages, so index it with [PkgId, declId]
// In addition, some decl is definitely declared in current package, so we only
// use declId instead of fullId to store it.
class ASTWriter {
public:
    ASTWriter(DiagnosticEngine& diag, const std::string& packageDepInfo, const ExportConfig& exportCfg,
        const CodeoManager& codeoManager);
    ~ASTWriter();

    // Add for codemp
    void SetSerializingCommon();

    /** Export external decls of a package AST to a buffer. */
    void ExportAST(const AST::PackageDecl& package) const;
    /** Pre-save serialized generic decls, inlinable function, default functions, constant vardecls. */
    void PreSaveFullExportDecls(AST::Package& package) const;
    void AST2FB(std::vector<uint8_t>& data, const AST::PackageDecl& package) const;

    // Save semaTypes, return TyIndex and construct savedTypeMap.
    FormattedIndex SaveType(Ptr<const AST::Ty> pType) const;
    void SetIsChirNow(bool isChirNow = false);

private:
    class ASTWriterImpl;
    std::unique_ptr<ASTWriterImpl> pImpl;
};

// Decls are indexed by fullId. So there are two methods to get decl:
// 1. if pkgId=currentPkgId, load decl based on declId.
// 2. others, find decl from codeoManager according to fullId.
class ASTLoader {
public:
    ASTLoader(std::vector<uint8_t>&& data, const std::string& fullPackageName, TypeManager& typeManager,
        const CodeoManager& codeoManager, const GlobalOptions& opts);
    // Not use default destructor because 'ASTLoaderImpl' is defined as forward decl in header.
    ~ASTLoader();

    OwnedPtr<AST::Package> LoadPackageDependencies() const;
    void LoadPackageDecls() const;
    std::unordered_set<std::string> LoadCachedTypeForPackage(
        const AST::Package& sourcePackage, const std::map<std::string, Ptr<AST::Decl>>& mangledName2DeclMap);
    std::string LoadPackageDepInfo() const;
    void LoadRefs() const;
    std::string GetImportedPackageName() const;
    void SetImportSourceCode(bool enable) const;
    const std::vector<std::string> GetDependentPackageNames() const;
    // Add for codemp
    void PreloadCommonPartOfPackage(AST::Package& pkg) const;
    std::string PreReadAndSetPackageName();
    std::vector<std::string> ReadFileNames() const;

    Ptr<AST::Ty> LoadType(FormattedIndex type) const;
    // A flag to avoid conflicts when we are reusing the AST serialiser from CHIR
    void SetIsChirNow(bool isChirNow = false);

private:
    class ASTLoaderImpl;
    OwnedPtr<ASTLoaderImpl> pImpl;
};
} // namespace Codira
#endif // CODIRA_MODULES_ASTSERIALIZATION_H
