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
 * This file declares the utils of TypeInfo.
 */

#ifndef CODIRA_CGTYPEINFO_H
#define CODIRA_CGTYPEINFO_H

#include "IRBuilder.h"

namespace Codira::CodeGen {
namespace CGTypeInfo {
llvm::Function* GenTypeInfoFns(llvm::FunctionType* fieldFnType, CGModule& cgMod, const std::string& funcName,
    const std::unordered_map<const CHIR::GenericType*, size_t>& outerGTIdxMap, const CHIR::Type& memberType,
    const std::string& memberName);
llvm::Constant* GenSuperFnOfTypeTemplate(CGModule& cgMod, const std::string& funcName, const CHIR::Type& superType,
    const std::vector<CHIR::GenericType*>& typeArgs);
llvm::Constant* GenFieldsFnsOfTypeTemplate(
    CGModule& cgMod, const std::string& funcPrefixName, const std::vector<llvm::Constant*>& fieldsFn);
llvm::Constant* GenFieldsFnsOfTypeTemplate(CGModule& cgMod, const std::string& funcPrefixName,
    const std::vector<CHIR::Type*>& fieldTypes,
    const std::unordered_map<const CHIR::GenericType*, size_t>& genericParamIndicesMap);
} // namespace CGTypeInfo
} // namespace Codira::CodeGen

#endif // CODIRA_CGTYPEINFO_H
