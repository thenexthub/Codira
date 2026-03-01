/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_UTILS_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_UTILS_H

#include "llvm_ark_interface.h"

#include <llvm/ADT/SmallSet.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>

namespace ark::llvmbackend::gc_utils {

enum class DerivedStatus { UNKNOWN, DERIVED, NOT_DERIVED };
bool IsDerived(llvm::Value *val);
DerivedStatus IsPHIDerived(llvm::PHINode *phi, llvm::SmallSet<llvm::Value *, 8U> *visited);
DerivedStatus IsDerivedImpl(llvm::Value *val, llvm::SmallSet<llvm::Value *, 8U> *visited = nullptr);
bool HasBeenGcRef(const llvm::Value *val, bool any = true);
void MarkAsNonMovable(llvm::Instruction *instruction);
bool IsNonMovable(const llvm::Value *value);

inline bool IsGcRefType(llvm::Type *type)
{
    return type->isPointerTy() && type->getPointerAddressSpace() == ark::llvmbackend::LLVMArkInterface::GC_ADDR_SPACE;
}

inline bool IsGcFunction(const llvm::Function &function)
{
    return function.hasGC() && function.getGC() == ark::llvmbackend::LLVMArkInterface::GC_STRATEGY;
}

inline bool IsFunctionSupplemental(const llvm::Function &function)
{
    if (function.isDeclaration()) {
        return true;
    }
    if (function.getName().equals(ark::llvmbackend::LLVMArkInterface::GC_SAFEPOINT_POLL_NAME)) {
        return true;
    }
    return false;
}

/// Returns true if a value is an integer comparison with 0.
inline bool IsNullCmp(const llvm::Value *val)
{
    auto cmp = llvm::dyn_cast<llvm::ICmpInst>(val);
    if (cmp == nullptr) {
        return false;
    }

    auto op0 = llvm::dyn_cast<llvm::Constant>(cmp->getOperand(0));
    auto op1 = llvm::dyn_cast<llvm::Constant>(cmp->getOperand(1));
    return (op0 != nullptr && op0->isNullValue()) || (op1 != nullptr && op1->isNullValue());
}

/// Returns true if a value is an instruction that allowed to use with escaped value.
inline bool IsAllowedEscapedUser(const llvm::Value *val)
{
    return llvm::isa<llvm::CastInst>(val) || IsNullCmp(val);
}

}  // namespace ark::llvmbackend::gc_utils

#endif  //  LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_UTILS_H
