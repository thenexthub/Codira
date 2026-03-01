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

#include "CGPkgContext.h"

#include "CGModule.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/Utils/ProfileRecorder.h"

namespace Codira::CodeGen {
CGPkgContext::CGPkgContext(CHIR::CHIRBuilder& chirBuilder, const CHIRData& chirData, const GlobalOptions& options,
    bool enableIncrement, const CachedMangleMap& cachedMangleMap)
    : chirBuilder(chirBuilder), chirData(chirData), options(options), enableIncrement(enableIncrement)
{
    cachedMangleMap.Dump();
    correctedCachedMangleMap.importedInlineDecls = cachedMangleMap.importedInlineDecls;
    correctedCachedMangleMap.newExternalDecls = cachedMangleMap.newExternalDecls;
    for (auto& incrRemovedDecl : cachedMangleMap.incrRemovedDecls) {
        if (FindCHIRGlobalValue(incrRemovedDecl)) {
            continue;
        }
        correctedCachedMangleMap.incrRemovedDecls.emplace(incrRemovedDecl);
    }
    correctedCachedMangleMap.Dump();

    CollectSubTypeMap();
}

CGPkgContext::~CGPkgContext() = default;

void CGPkgContext::Clear()
{
    for (auto& cgMod : cgMods) {
        cgMod->GetCGContext().Clear();
    }
    subTypeMap.clear();
    correctedCachedMangleMap.Clear();
    quickCHIRValues.Do([](std::unordered_map<std::string, CHIR::Value*>& object) { object.clear(); });
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    localizedSymbols.Do([](std::set<std::string>& object) { object.clear(); });
#endif
}

std::string CGPkgContext::GetCurrentPkgName() const
{
    auto curPackage = chirData.GetCurrentCHIRPackage();
    CODEC_NULLPTR_CHECK(curPackage);
    return curPackage->GetName();
}

CHIR::FuncBase* CGPkgContext::GetImplicitUsedFunc(const std::string& funcMangledName)
{
    auto funcs = chirData.GetImplicitFuncs();
    auto it = funcs.find(funcMangledName);
    CODEC_ASSERT(it != funcs.end());
    return it->second;
}

void CGPkgContext::AddCGModule(std::unique_ptr<CGModule>& cgMod)
{
    cgMods.emplace_back(std::move(cgMod));
}
const std::vector<std::unique_ptr<CGModule>>& CGPkgContext::GetCGModules()
{
    return cgMods;
}
std::vector<std::unique_ptr<llvm::Module>> CGPkgContext::ReleaseLLVMModules()
{
    std::vector<std::unique_ptr<llvm::Module>> llvmModules;
    std::for_each(cgMods.begin(), cgMods.end(), [&llvmModules](std::unique_ptr<CGModule>& cgMod) {
        llvmModules.emplace_back(cgMod->ReleaseLLVMModule());
        cgMod->GetCGContext().Clear();
    });
    cgMods.clear();
    return llvmModules;
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
void CGPkgContext::AddLocalizedSymbol(const std::string& symName)
{
    localizedSymbols.Do([&symName](std::set<std::string>& object) { object.emplace(symName); });
}
const std::set<std::string>& CGPkgContext::GetLocalizedSymbols()
{
    return localizedSymbols.Do(
        [](const std::set<std::string>& object) -> const std::set<std::string>& { return object; });
}

void CGPkgContext::CollectSubTypeMap()
{
    if (!GetGlobalOptions().CompileExecutable()) {
        return;
    }

    for (auto customTypeDef : GetCHIRPackage().GetAllCustomTypeDef()) {
        for (auto parentTy : customTypeDef->GetSuperTypesInCurDef()) {
            subTypeMap[parentTy].emplace(customTypeDef->GetType());
        }
    }
}

bool CGPkgContext::NeedOuterTypeInfo(const CHIR::ClassType& classType)
{
    if (!GetGlobalOptions().CompileExecutable()) {
        return true;
    }

    auto type = classType.GetClassDef()->GetType();
    if (!type->GetGenericArgs().empty()) {
        return true;
    }

    for (auto subTy : subTypeMap[type]) {
        if (!subTy->GetTypeArgs().empty()) {
            return true;
        }
        auto subClassType = DynamicCast<CHIR::ClassType*>(subTy);
        if (subClassType == nullptr) {
            continue;
        }
        if (NeedOuterTypeInfo(*subClassType)) {
            return true;
        }
    }
    return false;
}
#endif

CHIR::Value* CGPkgContext::FindCHIRGlobalValue(const std::string& mangledName)
{
    const CHIR::Package& capturedChirPkg = GetCHIRPackage();
    return quickCHIRValues.Do(
        [&capturedChirPkg, &mangledName](std::unordered_map<std::string, CHIR::Value*>& object) -> CHIR::Value* {
            if (object.empty()) {
                object.reserve(capturedChirPkg.GetGlobalFuncs().size() + capturedChirPkg.GetGlobalVars().size() +
                    capturedChirPkg.GetImportedVarAndFuncs().size());
                for (auto chirFunc : capturedChirPkg.GetGlobalFuncs()) {
                    object.emplace(chirFunc->GetIdentifierWithoutPrefix(), chirFunc);
                }
                for (auto chirGv : capturedChirPkg.GetGlobalVars()) {
                    object.emplace(chirGv->GetIdentifierWithoutPrefix(), chirGv);
                }
                for (auto importedValue : capturedChirPkg.GetImportedVarAndFuncs()) {
                    object.emplace(importedValue->GetIdentifierWithoutPrefix(), importedValue);
                }
            }

            if (auto target = object.find(mangledName); target != object.end()) {
                return target->second;
            } else {
                return nullptr;
            }
        });
}

const CHIR::Package& CGPkgContext::GetCHIRPackage() const
{
    auto chirPackage = chirData.GetCurrentCHIRPackage();
    CODEC_NULLPTR_CHECK(chirPackage);
    return *chirPackage;
}
} // namespace Codira::CodeGen
