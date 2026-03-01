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

#ifndef COMMON_DATA_H
#define COMMON_DATA_H

#include <string>
#include <vector>
#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/CHIR/Type/Type.h"
namespace Codira::CodeCheck {
const std::string ANY_TYPE = "ANY_TYPE";
const std::string NOT_CARE = "NOT_CARE";

struct AstFuncInfo {
    std::string funcName;
    std::string parentTy;
    std::vector<std::string> params;
    std::string returnTy;
    std::string pkgName;
    AstFuncInfo(const std::string &name, const std::string &parent, const std::vector<std::string> &params,
        const std::string &returnTy, const std::string &pkgName)
        : funcName(name), parentTy(parent), params(params), returnTy(returnTy), pkgName(pkgName)
    {}
    bool operator == (const AstFuncInfo &rhs) const
    {
        return (this->funcName == rhs.funcName && this->parentTy == rhs.parentTy && this->params == rhs.params &&
            this->returnTy == rhs.returnTy && this->pkgName == rhs.pkgName);
    }
};

struct CHIRFuncInfo {
    std::string funcName;
    Ptr<CHIR::CustomType> parentTy;
    Ptr<CHIR::FuncType> funcTy;
    std::string pkgName;
    CHIRFuncInfo() = default;
    CHIRFuncInfo(const std::string &name, Ptr<CHIR::CustomType> parentTy, Ptr<CHIR::FuncType> funcTy,
        const std::string &pkgName)
        : funcName(name), parentTy(parentTy), funcTy(funcTy), pkgName(pkgName)
    {
    }
    bool operator==(const CHIRFuncInfo &rhs) const
    {
        return (this->funcName == rhs.funcName && this->parentTy == rhs.parentTy && this->funcTy == rhs.funcTy &&
            this->pkgName == rhs.pkgName);
    }
};

} // namespace Codira::CodeCheck

#endif
