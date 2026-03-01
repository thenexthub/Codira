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

#ifndef CODIRA_CGGENERICTYPE_H
#define CODIRA_CGGENERICTYPE_H

#include "Base/CGTypes/CGType.h"
#include "CGContext.h"
#include "Codira/CHIR/Type/Type.h"

namespace Codira {
namespace CodeGen {
class CGGenericType : public CGType {
    friend class CGTypeMgr;

public:
    llvm::GlobalVariable* GetOrCreateTypeInfo() override;

private:
    CGGenericType() = delete;
    explicit CGGenericType(CGModule& cgMod, CGContext& cgCtx, const CHIR::Type& chirType)
        : CGType(cgMod, cgCtx, chirType)
    {
        CODEC_ASSERT(chirType.GetTypeKind() == CHIR::Type::TYPE_GENERIC);
    }
    std::vector<CHIR::Type*> upperBounds;
    llvm::Type* GenLLVMType() override;
    void GenContainedCGTypes() override;
    void CalculateSizeAndAlign() override;
    llvm::Constant* GenUpperBoundsOfGenericType(std::string& uniqueName);
};
} // namespace CodeGen
} // namespace Codira

#endif // CODIRA_CGGENERICTYPE_H
