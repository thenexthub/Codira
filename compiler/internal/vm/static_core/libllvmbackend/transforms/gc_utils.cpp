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

#include "gc_utils.h"
#include "transforms/transform_utils.h"

namespace {
/// Tag for instructions have a managed pointer type but the reference is not moved by GC
constexpr llvm::StringRef MD_NON_MOVABLE = "non-movable";
}  // namespace

namespace ark::llvmbackend::gc_utils {

bool IsDerived(llvm::Value *val)
{
    auto isDerived = IsDerivedImpl(val);
    ASSERT(isDerived != DerivedStatus::UNKNOWN);
    return isDerived == DerivedStatus::DERIVED;
}

DerivedStatus IsDerivedImpl(llvm::Value *val, llvm::SmallSet<llvm::Value *, 8U> *visited)
{
    auto phi = llvm::dyn_cast<llvm::PHINode>(val);
    if (phi != nullptr) {
        ASSERT(phi->getNumIncomingValues() > 0);
        if (visited == nullptr) {
            llvm::SmallSet<llvm::Value *, 8U> lvis;
            return IsPHIDerived(phi, &lvis);
        }
        return IsPHIDerived(phi, visited);
    }
    auto select = llvm::dyn_cast<llvm::SelectInst>(val);
    if (select != nullptr) {
        auto derived = IsDerivedImpl(select->getTrueValue(), visited);
        if (derived == DerivedStatus::UNKNOWN) {
            derived = IsDerivedImpl(select->getFalseValue(), visited);
        } else {
#ifndef NDEBUG
            auto derivedFalseBr = IsDerivedImpl(select->getFalseValue(), visited);
            ASSERT(derived == derivedFalseBr || derivedFalseBr == DerivedStatus::UNKNOWN);
#endif
        }
        return derived;
    }
    // NOTE: Workaround for situation when llvm do the following during optimization pipeline:
    // gep(ptr + const) => gep(nullptr + const) => gep(const)
    if (auto *constantExpr = llvm::dyn_cast<llvm::ConstantExpr>(val)) {
        if (constantExpr->getOpcode() == llvm::Instruction::IntToPtr) {
            return DerivedStatus::UNKNOWN;
        }
    }
    if (llvm::isa<llvm::IntToPtrInst>(val)) {
        return DerivedStatus::NOT_DERIVED;
    }
    if (llvm::isa<llvm::CastInst>(val)) {
        if (!val->getType()->isPointerTy()) {
            return DerivedStatus::NOT_DERIVED;
        }
        auto cast = llvm::cast<llvm::CastInst>(val);
        return IsDerivedImpl(cast->stripPointerCasts(), visited);
    }
    return llvm::isa<llvm::GetElementPtrInst>(val) ? DerivedStatus::DERIVED : DerivedStatus::NOT_DERIVED;
}

DerivedStatus IsPHIDerived(llvm::PHINode *phi, llvm::SmallSet<llvm::Value *, 8U> *visited)
{
    ASSERT(visited != nullptr);
    if (visited->find(phi) != visited->end()) {
        return DerivedStatus::UNKNOWN;
    }
    visited->insert(phi);
    auto result = DerivedStatus::UNKNOWN;
    for (auto &ival : phi->incoming_values()) {
        auto isIvalDerived = IsDerivedImpl(&(*ival), visited);
#ifndef NDEBUG
        if (result == DerivedStatus::UNKNOWN && isIvalDerived != DerivedStatus::UNKNOWN) {
            result = isIvalDerived;
        }
        ASSERT(isIvalDerived == result || isIvalDerived == DerivedStatus::UNKNOWN);
#else
        if (isIvalDerived != DerivedStatus::UNKNOWN) {
            return isIvalDerived;
        }
#endif
    }
    return result;
}

bool HasBeenGcRef(const llvm::Value *val, bool any)
{
    auto stripUntilRoot = [](const llvm::Value *toStrip) {
        auto cast = llvm::dyn_cast<llvm::CastInst>(toStrip);
        while (cast != nullptr && !IsGcRefType(toStrip->getType())) {
            toStrip = cast->getOperand(0);
            cast = llvm::dyn_cast<llvm::CastInst>(toStrip);
        }
        return toStrip;
    };

    auto root = stripUntilRoot(val);
    if (IsGcRefType(root->getType())) {
        return true;
    }

    // Roots consist only of non-GC references that are free of casts.
    llvm::SmallVector<const llvm::Value *, 8U> roots;
    llvm::SmallSet<const llvm::Value *, 8U> visited;
    roots.push_back(root);
    bool oneRef = false;
    while (!roots.empty()) {
        if (any && oneRef) {
            // Do not need to process further in ANY mode.
            return true;
        }
        root = roots.pop_back_val();
        if (!visited.insert(root).second) {
            continue;
        }
        if (!llvm::isa<llvm::PHINode, llvm::SelectInst>(root)) {
            if (!llvm::isa<llvm::Constant>(root) && !any) {
                // A non-phi and non-constant:
                // ANY mode should continue searching a GC reference
                // NOT-ANY mode treat it as a non GC-reference, therefore can return false.
                return false;
            }
            continue;
        }
        auto inst = llvm::cast<llvm::Instruction>(root);
        for (size_t i = llvm::isa<llvm::SelectInst>(inst) ? 1 : 0; i < inst->getNumOperands(); ++i) {
            root = stripUntilRoot(inst->getOperand(i));
            if (IsGcRefType(root->getType())) {
                oneRef = true;
            } else {
                roots.push_back(root);
            }
        }
    }
    return oneRef;
}

void MarkAsNonMovable(llvm::Instruction *instruction)
{
    ASSERT(IsGcRefType(instruction->getType()));
    instruction->setMetadata(MD_NON_MOVABLE, llvm::MDNode::get(instruction->getContext(), {}));
}

bool IsNonMovable(const llvm::Value *value)
{
    auto instruction = llvm::dyn_cast<const llvm::Instruction>(value);
    return instruction != nullptr && instruction->hasMetadata(MD_NON_MOVABLE);
}

}  // namespace ark::llvmbackend::gc_utils
