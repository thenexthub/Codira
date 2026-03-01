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

#ifndef CODIRA_CHIR_IMPLICIT_IMPORTED_FUNC_MGR_H
#define CODIRA_CHIR_IMPLICIT_IMPORTED_FUNC_MGR_H

#include "Codira/AST/Node.h"

namespace Codira::CHIR {
/**
 * The information structure of the imported functions that are implicitly called only in CodeGen.
 */
struct ImplicitImportedFunc {
    AST::ASTKind parentKind;
    std::string identifier{};
    std::string parentName{};
};

class ImplicitImportedFuncMgr {
public:
    enum class FuncKind : uint8_t { GENERIC, NONE_GENERIC };

    static ImplicitImportedFuncMgr& Instance() noexcept;
    void RegImplicitImportedFunc(const ImplicitImportedFunc& func, FuncKind funcKind) noexcept;
    std::vector<ImplicitImportedFunc> GetImplicitImportedFuncs(FuncKind funcKind);

    ImplicitImportedFuncMgr(ImplicitImportedFuncMgr&&) = delete;
    ImplicitImportedFuncMgr(const ImplicitImportedFuncMgr&) = delete;
    ImplicitImportedFuncMgr& operator=(ImplicitImportedFuncMgr&&) = delete;
    ImplicitImportedFuncMgr& operator=(const ImplicitImportedFuncMgr&) = delete;

private:
    ImplicitImportedFuncMgr() noexcept = default;
    ~ImplicitImportedFuncMgr() = default;

    /**
     * This vector is used to store imported generic function information.
     * These imported generic functions, which are called implicitly in CodeGen, are from the "std.core" package.
     * Their generic instances may be in other import packages.
     */
    std::vector<ImplicitImportedFunc> implicitImportedGenericFuncs{};
    /**
     * This vector is used to store imported non-generic function information.
     * These imported functions, which are called implicitly in CodeGen, are from the "std.core" package.
     * If the function has no parent class,
     * its parent class name is an empty string and its parentKind is "INVALID_DECL".
     */
    std::vector<ImplicitImportedFunc> implicitImportedNonGenericFuncs{};
};

class ImplicitImportedFuncRegister {
public:
    ImplicitImportedFuncRegister(const ImplicitImportedFunc& func, ImplicitImportedFuncMgr::FuncKind kind) noexcept
    {
        ImplicitImportedFuncMgr::Instance().RegImplicitImportedFunc(func, kind);
    }
    ~ImplicitImportedFuncRegister() = default;
};

#define REG_IMPLICIT_IMPORTED_NON_GENERIC_FUNC(outDeclKind, identifier, ...) \
static ImplicitImportedFuncRegister g_reg_##identifier##__VA_ARGS__( \
    {outDeclKind, #identifier, #__VA_ARGS__}, ImplicitImportedFuncMgr::FuncKind::NONE_GENERIC)

#define REG_IMPLICIT_IMPORTED_GENERIC_FUNC(outDeclKind, identifier, ...) \
static ImplicitImportedFuncRegister g_reg_##identifier##__VA_ARGS__( \
    {outDeclKind, #identifier, #__VA_ARGS__}, ImplicitImportedFuncMgr::FuncKind::GENERIC)
} // namespace Codira::CHIR
#endif // CODIRA_CHIR_IMPLICIT_IMPORTED_FUNC_MGR_H
