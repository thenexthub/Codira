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

#ifndef CODIRA_CODEGEN_IR_ATTRIBUTE_H
#define CODIRA_CODEGEN_IR_ATTRIBUTE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"

#include "Utils/CGUtils.h"

namespace Codira {
namespace CodeGen {
/********* Attribute adder for llvm::CallBase *********/

inline void AddAttributeAtIndex(llvm::CallBase* callBase, unsigned idx, llvm::Attribute attr)
{
    callBase->addAttributeAtIndex(idx, attr);
}
inline void AddAttributeAtIndex(llvm::CallBase* callBase, unsigned idx, llvm::Attribute::AttrKind kind)
{
    callBase->addAttributeAtIndex(idx, kind);
}

inline void AddParamAttr(llvm::CallBase* callBase, unsigned paramIdx, llvm::Attribute attr)
{
    AddAttributeAtIndex(callBase, llvm::AttributeList::FirstArgIndex + paramIdx, attr);
}
inline void AddParamAttr(llvm::CallBase* callBase, unsigned paramIdx, llvm::Attribute::AttrKind kind)
{
    AddAttributeAtIndex(callBase, llvm::AttributeList::FirstArgIndex + paramIdx, kind);
}

inline void AddRetAttr(llvm::CallBase* callBase, llvm::Attribute attr)
{
    AddAttributeAtIndex(callBase, llvm::AttributeList::ReturnIndex, attr);
}
inline void AddRetAttr(llvm::CallBase* callBase, llvm::Attribute::AttrKind kind)
{
    AddAttributeAtIndex(callBase, llvm::AttributeList::ReturnIndex, kind);
}

inline void AddSRetAttribute(llvm::CallBase* call)
{
    auto retType = call->arg_begin()->get()->getType();
    CODEC_ASSERT(retType->isPointerTy());
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto type = GetPointerElementType(retType);
    auto sRetAttr = llvm::Attribute::getWithStructRetType(call->getContext(), type);
    AddParamAttr(call, 0, sRetAttr);
#endif
}

inline void SetZExtAttrForCFunc(llvm::CallBase& call)
{
    if (call.getFunctionType()->getReturnType()->isIntegerTy(1)) {
        AddRetAttr(&call, llvm::Attribute::ZExt);
    }
    uint32_t i = 0;
    for (auto arg = call.arg_begin(); arg != call.arg_end(); arg++, i++) {
        if (arg->get()->getType()->isIntegerTy(1)) {
            call.addParamAttr(i, llvm::Attribute::ZExt);
        }
    }
}

/********* Attribute adder for llvm::Function *********/

inline void AddAttributeAtIndex(llvm::Function* func, unsigned idx, llvm::Attribute attr)
{
    func->addAttributeAtIndex(idx, attr);
}
inline void AddAttributeAtIndex(llvm::Function* func, unsigned idx, llvm::Attribute::AttrKind kind)
{
    func->addAttributeAtIndex(idx, llvm::Attribute::get(func->getContext(), kind));
}

inline void AddFnAttr(llvm::Function* func, llvm::Attribute attr)
{
    AddAttributeAtIndex(func, llvm::AttributeList::FunctionIndex, attr);
}
inline void AddFnAttr(llvm::Function* func, llvm::Attribute::AttrKind kind)
{
    AddAttributeAtIndex(func, llvm::AttributeList::FunctionIndex, kind);
}

inline void AddParamAttr(llvm::Function* func, unsigned paramIdx, llvm::Attribute attr)
{
    AddAttributeAtIndex(func, llvm::AttributeList::FirstArgIndex + paramIdx, attr);
}
inline void AddParamAttr(llvm::Function* func, unsigned paramIdx, llvm::Attribute::AttrKind kind)
{
    AddAttributeAtIndex(func, llvm::AttributeList::FirstArgIndex + paramIdx, kind);
}

inline void AddRetAttr(llvm::Function* func, llvm::Attribute attr)
{
    AddAttributeAtIndex(func, llvm::AttributeList::ReturnIndex, attr);
}
inline void AddRetAttr(llvm::Function* func, llvm::Attribute::AttrKind kind)
{
    AddAttributeAtIndex(func, llvm::AttributeList::ReturnIndex, kind);
}

/********* Attribute adder for llvm::Argument *********/

inline void AddSRetAttribute(llvm::Argument* value)
{
    CODEC_ASSERT(value->getType()->isPointerTy());
    auto type = GetPointerElementType(value->getType());
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto sRet = llvm::Attribute::getWithStructRetType(value->getContext(), type);
    value->addAttr(sRet);
#endif
}

inline void AddByValAttribute(llvm::Argument* value, uint64_t align = 8)
{
    CODEC_ASSERT(value->getType()->isPointerTy());
    CODEC_ASSERT(GetPointerElementType(value->getType())->isStructTy());
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto& llvmCtx = value->getContext();
    auto byValAttr = llvm::Attribute::getWithByValType(llvmCtx, GetPointerElementType(value->getType()));
    value->addAttr(byValAttr);
    auto alignAttr = llvm::Attribute::getWithAlignment(llvmCtx, llvm::Align(align));
    value->addAttr(alignAttr);
#endif
}

inline bool ShouldReturnVoid(const CHIR::Func& func)
{
    return Utils::In(func.GetFuncKind(),
        {
            CHIR::FuncKind::CLASS_CONSTRUCTOR,
            CHIR::FuncKind::PRIMAL_CLASS_CONSTRUCTOR,
            CHIR::FuncKind::STRUCT_CONSTRUCTOR,
            CHIR::FuncKind::PRIMAL_STRUCT_CONSTRUCTOR,
            CHIR::FuncKind::GLOBALVAR_INIT,
            CHIR::FuncKind::FINALIZER,
        });
}

inline void SetSRetAttrForStructReturnType(const CHIR::FuncType& chirFuncTy, llvm::Function& llvmFunc)
{
    bool shouldSetSRet = false;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    if (chirFuncTy.IsCFunc()) {
        // If the function is a CFunc, its SRet attribute has been determined whether to set.
        shouldSetSRet = llvmFunc.hasStructRetAttr();
    } else if (llvmFunc.getReturnType()->isVoidTy()) {
        // Unit and Struct return type will be changed into Void type in IR,
        // and the return value is moved to the first place of the parameters
        shouldSetSRet = true;
    }
#endif

    if (!llvmFunc.arg_empty() && shouldSetSRet) {
        llvmFunc.arg_begin()->addAttr(llvm::Attribute::NoAlias);
        AddSRetAttribute(llvmFunc.arg_begin());
    }
}
} // namespace CodeGen
} // namespace Codira

#endif // CODIRA_CODEGEN_IR_ATTRIBUTE_H
