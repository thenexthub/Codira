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

#ifndef CODIRA_CGCONTEXT_IMPL_H
#define CODIRA_CGCONTEXT_IMPL_H

#include <unordered_set>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"

#include "Base/CGTypes/CGType.h"
#include "Utils/CGCommonDef.h"
#include "Codira/CHIR/Type/Type.h"

namespace Codira {
namespace CodeGen {

class CGContextImpl {
    friend class CGContext;
    friend class CGType;
    friend class CGTupleType;

public:
    CGContextImpl() = default;
    ~CGContextImpl() = default;
    void Clear();

private:
    std::vector<CGType*> cgTypePool;
    std::unordered_map<CGType::TypeExtraInfo, std::unordered_map<const CHIR::Type*, CGType*>,
        CGType::TypeExtraInfoHasher> chirType2CGTypeMap;
    std::unordered_map<std::string, CGType*> chirTypeName2CGTypeMap;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    std::unordered_map<const llvm::Value*, llvm::Value*> valueAndBasePtrMap;
    std::unordered_set<llvm::Value*> nullableReference; // Record those i8(1)* which are nullable.
#endif
};

} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_CGCONTEXT_IMPL_H
