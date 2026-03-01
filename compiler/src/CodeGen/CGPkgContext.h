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

#ifndef CODIRA_CODEGEN_PACKAGE_CONTEXT_H
#define CODIRA_CODEGEN_PACKAGE_CONTEXT_H

#include <mutex>

#include "llvm/IR/Module.h"

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/IncrementalCompilation/CachedMangleMap.h"
#include "Codira/Option/Option.h"

namespace Codira {
namespace CodeGen {
class CGModule;
template <typename ObjectType> class ObjectLocker {
public:
    // Locks the associated object and calls the given function.
    template <typename Func> decltype(auto) Do(Func&& func)
    {
        std::unique_lock<std::mutex> lock(this->locker);
        return func(object);
    }

private:
    std::mutex locker;
    ObjectType object;
};

class CGPkgContext {
public:
    CGPkgContext(CHIR::CHIRBuilder& chirBuilder, const CHIRData& chirData, const GlobalOptions& options,
        bool enableIncrement, const CachedMangleMap& cachedMangleMap);
    ~CGPkgContext();
    void Clear();

    CHIR::CHIRBuilder& GetCHIRBuilder() const
    {
        return chirBuilder;
    }

    const CHIR::Package& GetCHIRPackage() const;

    std::string GetCurrentPkgName() const;

    const GlobalOptions& GetGlobalOptions() const
    {
        return options;
    }

    CHIR::FuncBase* GetImplicitUsedFunc(const std::string& funcMangledName);

    const CachedMangleMap& GetCachedMangleMap() const
    {
        return correctedCachedMangleMap;
    }

    bool IsIncrementEnabled() const
    {
        return enableIncrement;
    }

    bool IsCGParallelEnabled() const
    {
        return cgMods.size() > 1;
    }

    bool IsLineInfoEnabled() const
    {
        return options.enableCompileDebug || options.enableCoverage || options.displayLineInfo;
    }

    void AddCGModule(std::unique_ptr<CGModule>& cgMod);
    const std::vector<std::unique_ptr<CGModule>>& GetCGModules();
    std::vector<std::unique_ptr<llvm::Module>> ReleaseLLVMModules();

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    void AddLocalizedSymbol(const std::string& symName);
    const std::set<std::string>& GetLocalizedSymbols();
#endif

    void CollectSubTypeMap();
    bool NeedOuterTypeInfo(const CHIR::ClassType& classType);

    CHIR::Value* FindCHIRGlobalValue(const std::string& mangledName);

    CHIR::CHIRBuilder& chirBuilder;

private:
    const CHIRData& chirData;
    const GlobalOptions& options;
    const bool enableIncrement;
    CachedMangleMap correctedCachedMangleMap;

    std::vector<std::unique_ptr<CGModule>> cgMods;
    std::unordered_map<const CHIR::ClassType*, std::unordered_set<CHIR::Type*>> subTypeMap;
    // Container that support quick search for target global chirValue.
    ObjectLocker<std::unordered_map<std::string, CHIR::Value*>> quickCHIRValues;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    // The symbols, which need to be changed linkageType after the link.
    ObjectLocker<std::set<std::string>> localizedSymbols;
#endif
};
} // namespace CodeGen
} // namespace Codira
#endif
