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
 * This file declares some utility functions for interpreter module.
 */

#ifndef CODIRA_CHIR_INTERRETER_UTILS_H
#define CODIRA_CHIR_INTERRETER_UTILS_H

#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Interpreter/BCHIR.h"
#include "Codira/CHIR/Interpreter/InterpreterValue.h"
#include "Codira/CHIR/Interpreter/OpCodes.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Type/Type.h"

namespace Codira::CHIR::Interpreter {

OpCode PrimitiveTypeKind2OpCode(Type::TypeKind kind);
OpCode UnExprKind2OpCode(Codira::CHIR::ExprKind exprKind);
OpCode BinExprKind2OpCode(Codira::CHIR::ExprKind exprKind);
OpCode BinExprKindWitException2OpCode(Codira::CHIR::ExprKind exprKind);
IVal ByteCodeToIval(const Bchir::Definition& def, const Bchir& bchir, Bchir& topBchir);

template <bool OmitFirstArg = false>
std::string MangleMethodName(const std::string& methodName, const FuncType& funcTy)
{
    // T0D0: instead we can change SVTable so that the key is pair<std::string, Type>
    std::string res = methodName + "(";
    size_t start = 0;
    if constexpr (OmitFirstArg) {
        start = 1;
    }
    auto paramTys = funcTy.GetParamTypes();
    for (size_t i = start; i < paramTys.size(); i++) {
        res += paramTys[i]->ToString() + " ";
    }
    res += ")";
    return res;
}

}

#endif // CODIRA_CHIR_INTERRETER_BCHIR_H
