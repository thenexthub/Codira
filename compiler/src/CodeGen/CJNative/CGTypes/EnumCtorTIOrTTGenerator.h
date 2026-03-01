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
 * This file declares the class for determine the memory layout of Class/Interface.
 */

#ifndef CODIRA_CGENUMTYPELAYOUT_H
#define CODIRA_CGENUMTYPELAYOUT_H

#include "Base/CGTypes/CGType.h"

#include "Utils/CGCommonDef.h"
#include "Codira/CHIR/Type/EnumDef.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"

namespace Codira {
namespace CodeGen {
class EnumCtorTIOrTTGenerator {
public:
    explicit EnumCtorTIOrTTGenerator(CGModule& cgMod, const CHIR::EnumType& chirEnumType, std::size_t ctorIndex);

    void Emit();

private:
    void EmitForDynamicGI();
    void EmitForStaticGI();
    void EmitForConcrete();

    void GenerateNonGenericEnumCtorTypeInfo(llvm::GlobalVariable& ti);
    llvm::Constant* GenTypeArgsNumOfTypeInfo();
    llvm::Constant* GenTypeArgsOfTypeInfo();
    llvm::Constant* GenSourceGenericOfTypeInfo();

    void GenerateGenericEnumCtorTypeTemplate(llvm::GlobalVariable& tt);
    llvm::Constant* GenTypeArgsNumOfTypeTemplate();
    llvm::Constant* GenSuperFnOfTypeTemplate(const std::string& funcName);

private:
    CGModule& cgMod;
    CGContext& cgCtx;
    const CHIR::EnumType& chirEnumType;
    std::size_t ctorIndex;
};
} // namespace CodeGen
} // namespace Codira

#endif // CODIRA_CGENUMTYPELAYOUT_H
