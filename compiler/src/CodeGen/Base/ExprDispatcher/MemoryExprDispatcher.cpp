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

#include "Base/ExprDispatcher/ExprDispatcher.h"

#include <cinttypes>

#include "Base/AllocateImpl.h"
#include "Base/CGTypes/CGEnumType.h"
#include "Base/CHIRExprWrapper.h"
#include "CGModule.h"
#include "IRBuilder.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CodeGen {
llvm::Value* HandleLoadExpr(IRBuilder2& irBuilder, const CHIR::Load& load)
{
    auto& cgMod = irBuilder.GetCGModule();
    irBuilder.EmitLocation(CHIRExprWrapper(load));
    auto value = *(cgMod | load.GetLocation());
    CODEC_ASSERT(value.GetCGType()->GetOriginal().IsRef());
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto elementType = value.GetCGType()->GetPointerElementType()->GetLLVMType();
    if (load.GetLocation()->IsLocalVar() && elementType == CGType::GetRefType(irBuilder.GetLLVMContext())) {
        auto localVar = StaticCast<const CHIR::LocalVar*>(load.GetLocation());
        if (IsGetElementRefOfClass(*localVar->GetExpr(), irBuilder.GetCGContext().GetCHIRBuilder())) {
            auto getEleRefExpr = StaticCast<const CHIR::GetElementRef*>(localVar->GetExpr());
            auto locationCHIRType = DeRef(*getEleRefExpr->GetLocation()->GetType());
            if (locationCHIRType->IsClass() &&
                IsWeakRefClass(*StaticCast<CHIR::ClassType*>(locationCHIRType)->GetClassDef())) {
                auto addr = value.GetRawValue();
                auto base = irBuilder.GetCGContext().GetBasePtrOf(addr);
                CODEC_NULLPTR_CHECK(base);
                return irBuilder.CallGCReadWeakRef({base, addr});
            }
        }
    }
#endif
    return irBuilder.CreateLoad(value);
}

llvm::Value* HandleStoreExpr(IRBuilder2& irBuilder, const CHIR::Store& store)
{
    auto& cgMod = irBuilder.GetCGModule();
    auto value = store.GetValue();
    auto addr = store.GetLocation();
    if (auto node = DynamicCast<CHIR::LocalVar*>(value);
        node && node->GetExpr()->IsConstantNull() && value->GetType()->IsClass()) {
        return nullptr;
    } else if (auto var = DynamicCast<CHIR::LocalVar*>(addr); var && var->IsRetValue() && value->GetType()->IsUnit()) {
        // If stored location is function's ret of unit type, do nothing.
        return nullptr;
    }
    irBuilder.EmitLocation(CHIRExprWrapper(store), true);
    auto valueVal = *(cgMod | value);
    if (value->TestAttr(CHIR::Attribute::FOREIGN)) {
        CODEC_NULLPTR_CHECK(llvm::dyn_cast<llvm::Function>(valueVal.GetRawValue()));
        llvm::dyn_cast<llvm::Function>(valueVal.GetRawValue())
            ->addAttributeAtIndex(static_cast<unsigned>(llvm::AttributeList::FunctionIndex),
                llvm::Attribute::get(cgMod.GetLLVMContext(), CODE2C_ATTR));
    }
    return irBuilder.CreateStore(valueVal, *(cgMod | addr));
}

llvm::Value* GetElementRefOfOptionLikeT(
    IRBuilder2& irBuilder, const CHIR::EnumType& chirEnumType, bool isAntiOptionLike, llvm::Value* enumValue)
{
    auto ret = irBuilder.CreateEntryAlloca(irBuilder.getInt1Ty());
    auto payload = irBuilder.GetPayloadFromObject(enumValue);
    auto isRef = irBuilder.CreateTypeInfoIsReferenceCall(*chirEnumType.GetTypeArgs()[0]);
    auto [refBB, nonRefBB, endBB] = Vec2Tuple<3>(irBuilder.CreateAndInsertBasicBlocks({"ref", "nonRef", "end"}));
    irBuilder.CreateCondBr(isRef, refBB, nonRefBB);

    irBuilder.SetInsertPoint(refBB);
    auto refResult = isAntiOptionLike ? irBuilder.CallIntrinsicIsNonNull(irBuilder.CallGCRead({enumValue, payload}))
                                      : irBuilder.CallIntrinsicIsNull(irBuilder.CallGCRead({enumValue, payload}));
    irBuilder.CreateStore(refResult, ret);
    irBuilder.CreateBr(endBB);

    irBuilder.SetInsertPoint(nonRefBB);
    auto i1Ty = irBuilder.getInt1Ty();
    auto nonRefResult =
        irBuilder.LLVMIRBuilder2::CreateLoad(i1Ty, irBuilder.CreateBitCast(payload, i1Ty->getPointerTo(1U)));
    irBuilder.CreateStore(nonRefResult, ret);
    irBuilder.CreateBr(endBB);

    irBuilder.SetInsertPoint(endBB);
    return ret;
}

llvm::Value* HandleGetElementRef(IRBuilder2& irBuilder, const CHIR::GetElementRef& getEleRef)
{
    auto& cgMod = irBuilder.GetCGModule();
    auto base = getEleRef.GetLocation();
    CODEC_ASSERT(base->GetType()->IsRef() || base->GetType()->IsGeneric());
    irBuilder.EmitLocation(CHIRExprWrapper(getEleRef));
    llvm::Value* retValue = nullptr;
    if (auto t = DeRef(*base->GetType()); t && t->IsEnum()) {
        CODEC_ASSERT(!getEleRef.GetPath().empty() && getEleRef.GetPath()[0] == 0U);
        auto enumType = static_cast<const CHIR::EnumType*>(t);
        auto cgEnumType = static_cast<const CGEnumType*>(CGType::GetOrCreate(cgMod, enumType));
        auto layoutType = cgEnumType->GetLayoutType();
        auto value = irBuilder.CreateLoad(*(cgMod | base));
        if (cgEnumType->IsCommonEnum()) {
            auto payload = irBuilder.GetPayloadFromObject(value);
            auto tagLLVMType = llvm::Type::getInt32PtrTy(cgMod.GetLLVMContext(), 1U);
            retValue = irBuilder.CreateBitCast(payload, tagLLVMType);
        } else if (cgEnumType->IsOptionLikeNonRef()) {
            llvm::Value* payload = cgEnumType->GetSize().has_value()
                ? value
                : irBuilder.CreateBitCast(irBuilder.GetPayloadFromObject(value), layoutType->getPointerTo(1U));
            retValue = irBuilder.CreateStructGEP(layoutType, payload, 0);
        } else if (cgEnumType->IsOptionLikeRef()) {
            retValue = irBuilder.CreateEntryAlloca(irBuilder.getInt1Ty());
            auto tmp = cgEnumType->IsAntiOptionLike() ? irBuilder.CallIntrinsicIsNonNull(value)
                                                      : irBuilder.CallIntrinsicIsNull(value);
            irBuilder.CreateStore(tmp, retValue);
        } else if (cgEnumType->IsOptionLikeT()) {
            retValue = GetElementRefOfOptionLikeT(irBuilder, *enumType, cgEnumType->IsAntiOptionLike(), value);
        } else if (cgEnumType->IsAllAssociatedValuesAreNonRef()) {
            retValue = irBuilder.CreateBitCast(value, llvm::Type::getInt32PtrTy(cgMod.GetLLVMContext()));
        } else if (cgEnumType->IsZeroSizeEnum()) {
            retValue = irBuilder.CreateEntryAlloca(irBuilder.getInt32Ty());
            irBuilder.CreateStore(llvm::ConstantInt::get(irBuilder.getInt32Ty(), 0U), retValue);
        } else {
            CODEC_ABORT();
        }
    }
    if (!retValue) {
        retValue = irBuilder.CreateGEP(*(cgMod | base), getEleRef.GetPath()).GetRawValue();
    }

    auto opTy = DeRef(*getEleRef.GetOperand(0)->GetType());
    bool isAutoEnv =
        opTy->IsClass() && IsClosureConversionEnvClass(*StaticCast<CHIR::ClassType*>(opTy)->GetClassDef());
    bool isLambda = dynamic_cast<const CHIR::Func*>(&irBuilder.GetInsertCGFunction()->GetOriginal())->IsLambda();
    if (isLambda && irBuilder.GetCGContext().GetCompileOptions().enableCompileDebug && isAutoEnv) {
        irBuilder.CreateEnvDeclare(getEleRef, retValue);
    }
    return retValue;
}

void HandleStoreElementRef(IRBuilder2& irBuilder, const CHIR::StoreElementRef& storeElementRef)
{
    auto& cgMod = irBuilder.GetCGModule();
    auto rhs = storeElementRef.GetValue();
    auto lhs = storeElementRef.GetLocation();
    auto& path = storeElementRef.GetPath();
    CODEC_ASSERT(lhs->GetType()->IsRef() && !path.empty());
    auto value = cgMod | rhs;
    auto place = cgMod | lhs;
    auto srcCGType = CGType::GetOrCreate(cgMod, rhs->GetType());
    if (!srcCGType->GetSize() && IsThisArgOfStructMethod(*rhs)) {
        auto typeInfoOfSrc = irBuilder.CreateTypeInfo(srcCGType->GetOriginal());
        auto size = irBuilder.GetLayoutSize_32(srcCGType->GetOriginal());
        auto tmp = irBuilder.CallClassIntrinsicAlloc({typeInfoOfSrc, size});
        auto payloadPtr = irBuilder.GetPayloadFromObject(tmp);
        // Note: in this branch, it means:
        // - we are assigning a "struct" that doesn't begin with TypeInfo* to an address
        //   that should begin with TypeInfo*.
        // - we are in the scope of a struct instance method(without "$withTI" postfix),
        //   the "struct" mentioned above is the `this` parameter of the method.
        if (IsTypeContainsRef(srcCGType->GetLLVMType())) {
            irBuilder.CallGCWriteAgg({tmp, payloadPtr, value->GetRawValue(), size});
        } else {
            irBuilder.CreateMemCpy(payloadPtr, llvm::MaybeAlign(), value->GetRawValue(), llvm::MaybeAlign(), size);
        }
    } else if (auto node = DynamicCast<CHIR::LocalVar*>(rhs);
               node && node->GetExpr()->IsConstantNull() && rhs->GetType()->IsClass()) {
        // If the source is null or class, do nothing.
    } else if (auto var = DynamicCast<CHIR::LocalVar*>(lhs); var && var->IsRetValue() && rhs->GetType()->IsUnit()) {
        // If stored location is function's ret of unit type, do nothing.
    } else {
        irBuilder.EmitLocation(CHIRExprWrapper(storeElementRef));
        if (rhs->TestAttr(CHIR::Attribute::FOREIGN)) {
            llvm::cast<llvm::Function>(value->GetRawValue())
                ->addAttributeAtIndex(static_cast<unsigned>(llvm::AttributeList::FunctionIndex),
                    llvm::Attribute::get(cgMod.GetLLVMContext(), CODE2C_ATTR));
        }
        irBuilder.CreateStore(*value, irBuilder.CreateGEP(*place, path));
    }
}

llvm::Value* HandleMemoryExpression(IRBuilder2& irBuilder, const CHIR::Expression& chirExpr)
{
    CODEC_ASSERT(chirExpr.GetExprMajorKind() == CHIR::ExprMajorKind::MEMORY_EXPR);
    switch (chirExpr.GetExprKind()) {
        case CHIR::ExprKind::ALLOCATE: {
            auto& alloca = StaticCast<const CHIR::Allocate&>(chirExpr);
            // Opt: For the function that returns value by an `sret` argument,
            // we don't need to allocate another place to store the return value.
            if (auto parentFunc = alloca.GetTopLevelFunc();
                parentFunc && parentFunc->GetReturnValue() == alloca.GetResult()) {
                auto llvmFunc = irBuilder.GetCGModule().GetOrInsertCGFunction(parentFunc)->GetRawFunction();
                if (llvmFunc->hasStructRetAttr()) {
                    return llvmFunc->getArg(0);
                }
            }
            irBuilder.EmitLocation(CHIRExprWrapper(alloca));
            return GenerateAllocate(irBuilder, CHIRAllocateWrapper(alloca));
        }
        case CHIR::ExprKind::LOAD: {
            auto& load = StaticCast<const CHIR::Load&>(chirExpr);
            return HandleLoadExpr(irBuilder, load);
        }
        case CHIR::ExprKind::STORE: {
            auto& store = StaticCast<const CHIR::Store&>(chirExpr);
            return HandleStoreExpr(irBuilder, store);
        }
        case CHIR::ExprKind::GET_ELEMENT_REF: {
            auto& getEleRef = StaticCast<const CHIR::GetElementRef&>(chirExpr);
            return HandleGetElementRef(irBuilder, getEleRef);
        }
        case CHIR::ExprKind::STORE_ELEMENT_REF: {
            auto& storeElementRef = StaticCast<const CHIR::StoreElementRef&>(chirExpr);
            HandleStoreElementRef(irBuilder, storeElementRef);
            return nullptr;
        }
        default: {
            auto exprKindStr = std::to_string(static_cast<uint64_t>(chirExpr.GetExprKind()));
            CODEC_ASSERT_WITH_MSG(false, std::string("Unexpected CHIRExprKind: " + exprKindStr + "\n").c_str());
            return nullptr;
        }
    }
}
} // namespace Codira::CodeGen
