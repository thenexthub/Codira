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

#ifndef CODIRA_CGCLASSTYPE_H
#define CODIRA_CGCLASSTYPE_H

#include "Base/CGTypes/CGCustomType.h"
#include "Codira/CHIR/Type/Type.h"

namespace Codira {
namespace CodeGen {
class CGClassType : public CGCustomType {
    friend class CGTypeMgr;

public:
    llvm::StructType* GetLayoutType() const
    {
        return layoutType;
    }

    size_t GetNumOfAllFields() const
    {
        return numOfAllFields;
    }

protected:
    llvm::Type* GenLLVMType() override;
    void GenContainedCGTypes() override;

    llvm::Constant* GenSuperFnOfTypeTemplate() override;
    llvm::Constant* GenFinalizerOfTypeTemplate() override;
    void PreActionOfGenTypeInfo() override;
    void PreActionOfGenTypeTemplate() override;
    void PostActionOfGenTypeInfo() override;

private:
    CGClassType() = delete;

    explicit CGClassType(CGModule& cgMod, CGContext& cgCtx, const CHIR::ClassType& chirType);

    llvm::Constant* GenSuperOfTypeInfo() override;
    llvm::Constant* GenSourceGenericOfTypeInfo() override;
    void CalculateSizeAndAlign() override;

private:
    size_t numOfAllFields{0}; // including the fields inherited
};
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_CGCLASSTYPE_H
