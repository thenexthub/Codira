//===- WebAssembly.cpp ----------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"

using namespace language::Core;
using namespace language::Core::CodeGen;

//===----------------------------------------------------------------------===//
// WebAssembly ABI Implementation
//
// This is a very simple ABI that relies a lot on DefaultABIInfo.
//===----------------------------------------------------------------------===//

class WebAssemblyABIInfo final : public ABIInfo {
  DefaultABIInfo defaultInfo;
  WebAssemblyABIKind Kind;

public:
  explicit WebAssemblyABIInfo(CodeGen::CodeGenTypes &CGT,
                              WebAssemblyABIKind Kind)
      : ABIInfo(CGT), defaultInfo(CGT), Kind(Kind) {}

private:
  ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType Ty) const;

  // DefaultABIInfo's classifyReturnType and classifyArgumentType are
  // non-virtual, but computeInfo and EmitVAArg are virtual, so we
  // overload them.
  void computeInfo(CGFunctionInfo &FI) const override {
    if (!getCXXABI().classifyReturnType(FI))
      FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
    for (auto &Arg : FI.arguments())
      Arg.info = classifyArgumentType(Arg.type);
  }

  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override;
};

class WebAssemblyTargetCodeGenInfo final : public TargetCodeGenInfo {
public:
  explicit WebAssemblyTargetCodeGenInfo(CodeGen::CodeGenTypes &CGT,
                                        WebAssemblyABIKind K)
      : TargetCodeGenInfo(std::make_unique<WebAssemblyABIInfo>(CGT, K)) {
    SwiftInfo =
        std::make_unique<SwiftABIInfo>(CGT, /*SwiftErrorInRegister=*/false);
  }

  void setTargetAttributes(const Decl *D, toolchain::GlobalValue *GV,
                           CodeGen::CodeGenModule &CGM) const override {
    TargetCodeGenInfo::setTargetAttributes(D, GV, CGM);
    if (const auto *FD = dyn_cast_or_null<FunctionDecl>(D)) {
      if (const auto *Attr = FD->getAttr<WebAssemblyImportModuleAttr>()) {
        toolchain::Function *Fn = cast<toolchain::Function>(GV);
        toolchain::AttrBuilder B(GV->getContext());
        B.addAttribute("wasm-import-module", Attr->getImportModule());
        Fn->addFnAttrs(B);
      }
      if (const auto *Attr = FD->getAttr<WebAssemblyImportNameAttr>()) {
        toolchain::Function *Fn = cast<toolchain::Function>(GV);
        toolchain::AttrBuilder B(GV->getContext());
        B.addAttribute("wasm-import-name", Attr->getImportName());
        Fn->addFnAttrs(B);
      }
      if (const auto *Attr = FD->getAttr<WebAssemblyExportNameAttr>()) {
        toolchain::Function *Fn = cast<toolchain::Function>(GV);
        toolchain::AttrBuilder B(GV->getContext());
        B.addAttribute("wasm-export-name", Attr->getExportName());
        Fn->addFnAttrs(B);
      }
    }

    if (auto *FD = dyn_cast_or_null<FunctionDecl>(D)) {
      toolchain::Function *Fn = cast<toolchain::Function>(GV);
      if (!FD->doesThisDeclarationHaveABody() && !FD->hasPrototype())
        Fn->addFnAttr("no-prototype");
    }
  }

  /// Return the WebAssembly externref reference type.
  virtual toolchain::Type *getWasmExternrefReferenceType() const override {
    return toolchain::Type::getWasm_ExternrefTy(getABIInfo().getVMContext());
  }
  /// Return the WebAssembly funcref reference type.
  virtual toolchain::Type *getWasmFuncrefReferenceType() const override {
    return toolchain::Type::getWasm_FuncrefTy(getABIInfo().getVMContext());
  }
};

/// Classify argument of given type \p Ty.
ABIArgInfo WebAssemblyABIInfo::classifyArgumentType(QualType Ty) const {
  Ty = useFirstFieldIfTransparentUnion(Ty);

  if (isAggregateTypeForABI(Ty)) {
    // Records with non-trivial destructors/copy-constructors should not be
    // passed by value.
    if (auto RAA = getRecordArgABI(Ty, getCXXABI()))
      return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace(),
                                     RAA == CGCXXABI::RAA_DirectInMemory);
    // Ignore empty structs/unions.
    if (isEmptyRecord(getContext(), Ty, true))
      return ABIArgInfo::getIgnore();
    // Lower single-element structs to just pass a regular value. TODO: We
    // could do reasonable-size multiple-element structs too, using getExpand(),
    // though watch out for things like bitfields.
    if (const Type *SeltTy = isSingleElementStruct(Ty, getContext()))
      return ABIArgInfo::getDirect(CGT.ConvertType(QualType(SeltTy, 0)));
    // For the experimental multivalue ABI, fully expand all other aggregates
    if (Kind == WebAssemblyABIKind::ExperimentalMV) {
      const RecordType *RT = Ty->getAs<RecordType>();
      assert(RT);
      bool HasBitField = false;
      for (auto *Field :
           RT->getOriginalDecl()->getDefinitionOrSelf()->fields()) {
        if (Field->isBitField()) {
          HasBitField = true;
          break;
        }
      }
      if (!HasBitField)
        return ABIArgInfo::getExpand();
    }
  }

  // Otherwise just do the default thing.
  return defaultInfo.classifyArgumentType(Ty);
}

ABIArgInfo WebAssemblyABIInfo::classifyReturnType(QualType RetTy) const {
  if (isAggregateTypeForABI(RetTy)) {
    // Records with non-trivial destructors/copy-constructors should not be
    // returned by value.
    if (!getRecordArgABI(RetTy, getCXXABI())) {
      // Ignore empty structs/unions.
      if (isEmptyRecord(getContext(), RetTy, true))
        return ABIArgInfo::getIgnore();
      // Lower single-element structs to just return a regular value. TODO: We
      // could do reasonable-size multiple-element structs too, using
      // ABIArgInfo::getDirect().
      if (const Type *SeltTy = isSingleElementStruct(RetTy, getContext()))
        return ABIArgInfo::getDirect(CGT.ConvertType(QualType(SeltTy, 0)));
      // For the experimental multivalue ABI, return all other aggregates
      if (Kind == WebAssemblyABIKind::ExperimentalMV)
        return ABIArgInfo::getDirect();
    }
  }

  // Otherwise just do the default thing.
  return defaultInfo.classifyReturnType(RetTy);
}

RValue WebAssemblyABIInfo::EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                                     QualType Ty, AggValueSlot Slot) const {
  bool IsIndirect = isAggregateTypeForABI(Ty) &&
                    !isEmptyRecord(getContext(), Ty, true) &&
                    !isSingleElementStruct(Ty, getContext());
  return emitVoidPtrVAArg(CGF, VAListAddr, Ty, IsIndirect,
                          getContext().getTypeInfoInChars(Ty),
                          CharUnits::fromQuantity(4),
                          /*AllowHigherAlign=*/true, Slot);
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createWebAssemblyTargetCodeGenInfo(CodeGenModule &CGM,
                                            WebAssemblyABIKind K) {
  return std::make_unique<WebAssemblyTargetCodeGenInfo>(CGM.getTypes(), K);
}
