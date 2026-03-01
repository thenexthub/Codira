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
 * This file declares the CHIRMangle.
 */

#ifndef CODIRA_MANGLE_CHIRMANGLER_H
#define CODIRA_MANGLE_CHIRMANGLER_H

#include "Codira/Mangle/BaseMangler.h"

namespace Codira::CHIR {
/**
 * Name mangle class
 * Class CHIRMangler is designed to manage mangling rules during CHIR compilation.
 *
 */
class CHIRMangler : public BaseMangler {
public:
    /**
     * @brief The constructor of class CHIRMangler.
     *
     * @param compileTest The variable to enable compile test.
     * @return CHIRMangler The instance of CHIRMangler.
     */
    CHIRMangler(bool compileTest) : BaseMangler(), enableCompileTest(compileTest){};

    /**
     * @brief The constructor of class CHIRMangler.
     *
     * @param delimiter The delimiter of module.
     * @param compileTest The variable to enable compile test.
     * @return CHIRMangler The instance of CHIRMangler.
     */
    CHIRMangler(const std::string& delimiter, bool compileTest)
        : BaseMangler(delimiter), enableCompileTest(compileTest){};

    /**
     * @brief The destructor of class CHIRMangler.
     */
    virtual ~CHIRMangler() = default;

protected:
    /**
     * @brief Mangle the signature of CFunc.
     *        eg: the signature of CFunc<(Int64, Bool)->Float64> is "dlb".
     *
     * @param cFuncTy Indicates it is the sema type.
     * @return std::string The mangled signature.
     */
    std::string MangleCFuncSignature(const AST::FuncTy& cFuncTy) const;

private:
    std::optional<std::string> MangleEntryFunction(const Codira::AST::FuncDecl& funcDecl) const override;

    bool enableCompileTest;
};

} // namespace Codira::CHIR
#endif // CODIRA_MANGLE_CHIRMANGLER_H
