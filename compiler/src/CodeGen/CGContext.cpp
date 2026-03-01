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

#include "CGContext.h"

#include "CGContextImpl.h"
#include "CGModule.h"
#include "Codira/Option/Option.h"

namespace Codira {
namespace CodeGen {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
CGContext::CGContext(const SubCHIRPackage& subCHIRPackage, CGPkgContext& cgPkgContext)
    : cgPkgContext(cgPkgContext), subCHIRPackage(subCHIRPackage), llvmContext(nullptr)
{
    llvmContext = new llvm::LLVMContext(); // This `llvmContext` will be released in the de-constructor of `CGModule`.
    llvmContext->setOpaquePointers(cgPkgContext.GetGlobalOptions().enableOpaque);
    impl = std::make_unique<CGContextImpl>();
}
#endif

CGContext::~CGContext() = default;

llvm::StructType* CGContext::GetCodeStringType() const
{
    auto p1i8Type = llvm::Type::getInt8PtrTy(*llvmContext, 1u);
    auto int32Type = llvm::Type::getInt32Ty(*llvmContext);
    const std::string stringTypeStr = "record.std.core:String";
    if (auto stringType = llvm::StructType::getTypeByName(*llvmContext, stringTypeStr)) {
        return stringType;
    } else {
        return llvm::StructType::create(*llvmContext, {p1i8Type, int32Type, int32Type}, stringTypeStr);
    }
}

void CGContext::Add2CGTypePool(CGType* cgType)
{
    impl->cgTypePool.emplace_back(cgType);
}

void CGContext::Clear()
{
    impl->Clear();
    codeStrings.clear();
    generatedStructType.clear();
    globalsOfCompileUnit.clear();
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    llvmUsedGVs.clear();
    subCHIRPackage.Clear();
    callBasesToInline.clear();
    callBasesToReplace.clear();
    debugLocOfRetExpr.clear();
#endif
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
llvm::Value* CGContext::GetBasePtrOf(llvm::Value* val) const
{
    if (auto it = impl->valueAndBasePtrMap.find(val); it != impl->valueAndBasePtrMap.end()) {
        return it->second;
    }
    return nullptr;
}

void CGContext::SetBasePtr(const llvm::Value* val, llvm::Value* basePtr)
{
    impl->valueAndBasePtrMap[val] = basePtr;
}
#endif

void CGContext::PushUnwindBlockStack(llvm::BasicBlock* unwindBlock)
{
    unwindBlockStack.push(unwindBlock);
}

std::optional<llvm::BasicBlock*> CGContext::TopUnwindBlockStack() const
{
    if (unwindBlockStack.empty() || unwindBlockStack.top() == nullptr) {
        return std::nullopt;
    } else {
        return unwindBlockStack.top();
    }
}

void CGContext::PopUnwindBlockStack()
{
    if (!unwindBlockStack.empty()) {
        unwindBlockStack.pop();
    }
}

void CGContext::AddGeneratedStructType(const std::string& structTypeName)
{
    CODEC_ASSERT(!structTypeName.empty());
    generatedStructType.emplace(structTypeName);
}
const std::set<std::string>& CGContext::GetGeneratedStructType() const
{
    return generatedStructType;
}
bool CGContext::IsGeneratedStructType(const std::string& structTypeName)
{
    return generatedStructType.find(structTypeName) != generatedStructType.end();
}

void CGContext::AddGlobalsOfCompileUnit(const std::string& globalsName)
{
    globalsOfCompileUnit.emplace(globalsName);
}

bool CGContext::IsGlobalsOfCompileUnit(const std::string& globalsName)
{
    return globalsOfCompileUnit.find(globalsName) != globalsOfCompileUnit.end();
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
void CGContext::AddNullableReference(llvm::Value* value)
{
    (void)impl->nullableReference.emplace(value);
}
#endif
} // namespace CodeGen
} // namespace Codira
