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

#ifndef CODIRA_CGARRAYTYPE_H
#define CODIRA_CGARRAYTYPE_H

#include "Base/CGTypes/CGType.h"
#include "Codira/CHIR/Type/Type.h"

namespace Codira {
namespace CodeGen {

class CGArrayType : public CGType {
    friend class CGTypeMgr;

public:
    CGType* GetElementCGType() const
    {
        auto tmp = GetContainedTypes();
        CODEC_ASSERT(tmp.size() == 1);
        return tmp[0];
    }
    llvm::Type* GetLayoutType() const
    {
        return layoutType;
    }

    static llvm::StructType* GenerateArrayLayoutTypeInfo(
        CGContext& cgCtx, const std::string& layoutName, llvm::Type* elemType);
    static llvm::Type* GenerateArrayLayoutType(CGModule& cgMod, const CHIR::RawArrayType& arrTy);
    static bool IsRefArray(const CGType& elemType);
    static llvm::StructType* GenerateRefArrayLayoutType(CGContext& cgCtx);
    static std::string GetTypeNameByArrayType(CGModule& cgMod, const CHIR::RawArrayType& arrTy);
    static std::string GetTypeNameByArrayElementType(CGModule& cgMod, CHIR::Type& elemType);

protected:
    llvm::Type* GenLLVMType() override;
    void GenContainedCGTypes() override;

private:
    CGArrayType() = delete;

    explicit CGArrayType(CGModule& cgMod, CGContext& cgCtx, const CHIR::RawArrayType& chirType)
        : CGType(cgMod, cgCtx, chirType)
    {
    }

    llvm::Constant* GenSourceGenericOfTypeInfo() override;
    llvm::Constant* GenTypeArgsNumOfTypeInfo() override;
    llvm::Constant* GenTypeArgsOfTypeInfo() override;
    llvm::Constant* GenSuperOfTypeInfo() override;

    void CalculateSizeAndAlign() override;
};
} // namespace CodeGen
} // namespace Codira

#endif // CODIRA_CGARRAYTYPE_H
