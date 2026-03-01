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

#ifndef CODIRA_CGCUSTOMTYPE_H
#define CODIRA_CGCUSTOMTYPE_H

#include "Base/CGTypes/CGType.h"

namespace Codira {
namespace CodeGen {
class CGCustomType : public CGType {
public:
    static std::vector<llvm::Constant*> GenTypeInfoConstantVectorForTypes(
        CGModule& cgMod, const std::vector<CHIR::Type*>& chirTypes);
    static llvm::Constant* GenTypeInfoArray(
        CGModule& cgMod, std::string name, std::vector<llvm::Constant*> constants, const std::string_view& attr);
    static llvm::Constant* GenOffsetsArray(CGModule& cgMod, std::string name, llvm::StructType* layoutType);

protected:
    CGCustomType(
        CGModule& cgMod, CGContext& cgCtx, const CHIR::Type& chirType, CGTypeKind cgTypeKind = CGTypeKind::OTHERS);

    llvm::Constant* GenFieldsNumOfTypeInfo() override;
    llvm::Constant* GenFieldsOfTypeInfo() override;
    llvm::Constant* GenOffsetsOfTypeInfo() override;
    llvm::Constant* GenSourceGenericOfTypeInfo() override;
    llvm::Constant* GenTypeArgsNumOfTypeInfo() override;
    llvm::Constant* GenTypeArgsOfTypeInfo() override;
    llvm::Constant* GenReflectionOfTypeInfo() override;

    llvm::Constant* GenNameOfTypeTemplate();
    llvm::Constant* GenKindOfTypeTemplate();
    llvm::Constant* GenTypeArgsNumOfTypeTemplate();

    bool IsSized() const;

    virtual void PreActionOfGenTypeTemplate() {}
    virtual void PostActionOfGenTypeTemplate() {}
    virtual llvm::Constant* GenFieldsNumOfTypeTemplate();
    virtual llvm::Constant* GenFieldsFnsOfTypeTemplate();
    virtual llvm::Constant* GenSuperFnOfTypeTemplate();
    virtual llvm::Constant* GenFinalizerOfTypeTemplate();

private:
    CGCustomType() = delete;
    void GenTypeTemplate() override;
};
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_CGCUSTOMTYPE_H
