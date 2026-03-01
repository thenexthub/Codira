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

#ifndef CODIRA_CODEGEN_CHIRSPLITTER_H
#define CODIRA_CODEGEN_CHIRSPLITTER_H

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace Codira {
namespace CHIR {
class CustomTypeDef;
class ClassDef;
class EnumDef;
class StructDef;
class ExtendDef;
class GlobalVar;
class Value;
class FuncBase;
class Func;
class ImportedFunc;
}
namespace CodeGen {
class CGPkgContext;

struct ChirTypeDefCmp {
    bool operator()(const CHIR::CustomTypeDef* lhs, const CHIR::CustomTypeDef* rhs) const;
};

struct ChirValueCmp {
    bool operator()(const CHIR::Value* lhs, const CHIR::Value* rhs) const;
};

struct SubCHIRPackage {
    bool mainModule = false;
    std::size_t subCHIRPackageIdx;
    std::size_t exprNumInChirFuncs;
    std::size_t splitNum;
    std::set<CHIR::CustomTypeDef*, ChirTypeDefCmp> chirCustomDefs;
    std::set<CHIR::GlobalVar*, ChirValueCmp> chirGVs;
    std::set<CHIR::Func*, ChirValueCmp> chirFuncs;
    std::set<CHIR::ImportedFunc*, ChirValueCmp> chirForeigns;
    std::set<CHIR::ImportedFunc*, ChirValueCmp> chirImportedCFuncs;

    explicit SubCHIRPackage(std::size_t splitNum);
    void Clear();
};

class CHIRSplitter {
public:
    explicit CHIRSplitter(const CGPkgContext& cgPkgCtx);

    std::vector<SubCHIRPackage> SplitCHIRPackage();

private:
    struct SubCHIRPackagesCache {
        std::optional<std::size_t> splitNum = std::nullopt;
        std::map<std::string, std::size_t> classesCache;
        std::map<std::string, std::size_t> enumsCache;
        std::map<std::string, std::size_t> structsCache;
        std::map<std::string, std::size_t> extendDefCache;
        std::map<std::string, std::size_t> gvsCache;
        std::map<std::string, std::size_t> funcsCache;
        std::map<std::string, std::size_t> foreignsCache;
        std::map<std::string, std::size_t> importedCFuncsCache;
    };

    void CalcSplitsNum();

    void SplitCHIRFuncs(std::vector<SubCHIRPackage>& subCHIRPackages);
    void SplitCHIRClasses(std::vector<SubCHIRPackage>& subCHIRPackages);
    void SplitCHIREnums(std::vector<SubCHIRPackage>& subCHIRPackages);
    void SplitCHIRStructs(std::vector<SubCHIRPackage>& subCHIRPackages);
    void SplitCHIRExtends(std::vector<SubCHIRPackage>& subCHIRPackages);
    void SplitCHIRGlobalVars(std::vector<SubCHIRPackage>& subCHIRPackages);
    void SplitCHIRImportedCFuncs(std::vector<SubCHIRPackage>& subCHIRPackages);

    unsigned long FindIdxInCache(const std::string& key);

    void LoadSubCHIRPackagesInfo();
    void SaveSubCHIRPackagesInfo();

    const CGPkgContext& cgPkgCtx;
    std::size_t splitNum;
    std::size_t index;

    SubCHIRPackagesCache subCHIRPackagesCache;
};
} // namespace CodeGen
} // namespace Codira
#endif
