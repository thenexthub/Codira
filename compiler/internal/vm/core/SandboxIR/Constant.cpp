//===- Constant.cpp - The Constant classes of Sandbox IR ------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/SandboxIR/Constant.h"
#include "vm/core/SandboxIR/BasicBlock.h"
#include "vm/core/SandboxIR/Context.h"
#include "vm/core/SandboxIR/Function.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core::sandboxir {

#ifndef NDEBUG
void Constant::dumpOS(raw_ostream &OS) const {
  dumpCommonPrefix(OS);
  dumpCommonSuffix(OS);
}
#endif // NDEBUG

ConstantInt *ConstantInt::getTrue(Context &Ctx) {
  auto *LLVMC = toolchain::ConstantInt::getTrue(Ctx.LLVMCtx);
  return cast<ConstantInt>(Ctx.getOrCreateConstant(LLVMC));
}
ConstantInt *ConstantInt::getFalse(Context &Ctx) {
  auto *LLVMC = toolchain::ConstantInt::getFalse(Ctx.LLVMCtx);
  return cast<ConstantInt>(Ctx.getOrCreateConstant(LLVMC));
}
ConstantInt *ConstantInt::getBool(Context &Ctx, bool V) {
  auto *LLVMC = toolchain::ConstantInt::getBool(Ctx.LLVMCtx, V);
  return cast<ConstantInt>(Ctx.getOrCreateConstant(LLVMC));
}
Constant *ConstantInt::getTrue(Type *Ty) {
  auto *LLVMC = toolchain::ConstantInt::getTrue(Ty->LLVMTy);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}
Constant *ConstantInt::getFalse(Type *Ty) {
  auto *LLVMC = toolchain::ConstantInt::getFalse(Ty->LLVMTy);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}
Constant *ConstantInt::getBool(Type *Ty, bool V) {
  auto *LLVMC = toolchain::ConstantInt::getBool(Ty->LLVMTy, V);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}
Constant *ConstantInt::get(Type *Ty, uint64_t V, bool IsSigned) {
  auto *LLVMC = toolchain::ConstantInt::get(Ty->LLVMTy, V, IsSigned);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}
ConstantInt *ConstantInt::get(IntegerType *Ty, uint64_t V, bool IsSigned) {
  auto *LLVMC = toolchain::ConstantInt::get(Ty->LLVMTy, V, IsSigned);
  return cast<ConstantInt>(Ty->getContext().getOrCreateConstant(LLVMC));
}
ConstantInt *ConstantInt::getSigned(IntegerType *Ty, int64_t V) {
  auto *LLVMC =
      toolchain::ConstantInt::getSigned(cast<toolchain::IntegerType>(Ty->LLVMTy), V);
  return cast<ConstantInt>(Ty->getContext().getOrCreateConstant(LLVMC));
}
Constant *ConstantInt::getSigned(Type *Ty, int64_t V) {
  auto *LLVMC = toolchain::ConstantInt::getSigned(Ty->LLVMTy, V);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}
ConstantInt *ConstantInt::get(Context &Ctx, const APInt &V) {
  auto *LLVMC = toolchain::ConstantInt::get(Ctx.LLVMCtx, V);
  return cast<ConstantInt>(Ctx.getOrCreateConstant(LLVMC));
}
ConstantInt *ConstantInt::get(IntegerType *Ty, StringRef Str, uint8_t Radix) {
  auto *LLVMC =
      toolchain::ConstantInt::get(cast<toolchain::IntegerType>(Ty->LLVMTy), Str, Radix);
  return cast<ConstantInt>(Ty->getContext().getOrCreateConstant(LLVMC));
}
Constant *ConstantInt::get(Type *Ty, const APInt &V) {
  auto *LLVMC = toolchain::ConstantInt::get(Ty->LLVMTy, V);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}
IntegerType *ConstantInt::getIntegerType() const {
  auto *LLVMTy = cast<toolchain::ConstantInt>(Val)->getIntegerType();
  return cast<IntegerType>(Ctx.getType(LLVMTy));
}

bool ConstantInt::isValueValidForType(Type *Ty, uint64_t V) {
  return toolchain::ConstantInt::isValueValidForType(Ty->LLVMTy, V);
}
bool ConstantInt::isValueValidForType(Type *Ty, int64_t V) {
  return toolchain::ConstantInt::isValueValidForType(Ty->LLVMTy, V);
}

Constant *ConstantFP::get(Type *Ty, double V) {
  auto *LLVMC = toolchain::ConstantFP::get(Ty->LLVMTy, V);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}

Constant *ConstantFP::get(Type *Ty, const APFloat &V) {
  auto *LLVMC = toolchain::ConstantFP::get(Ty->LLVMTy, V);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}

Constant *ConstantFP::get(Type *Ty, StringRef Str) {
  auto *LLVMC = toolchain::ConstantFP::get(Ty->LLVMTy, Str);
  return Ty->getContext().getOrCreateConstant(LLVMC);
}

ConstantFP *ConstantFP::get(const APFloat &V, Context &Ctx) {
  auto *LLVMC = toolchain::ConstantFP::get(Ctx.LLVMCtx, V);
  return cast<ConstantFP>(Ctx.getOrCreateConstant(LLVMC));
}

Constant *ConstantFP::getNaN(Type *Ty, bool Negative, uint64_t Payload) {
  auto *LLVMC = toolchain::ConstantFP::getNaN(Ty->LLVMTy, Negative, Payload);
  return cast<Constant>(Ty->getContext().getOrCreateConstant(LLVMC));
}
Constant *ConstantFP::getQNaN(Type *Ty, bool Negative, APInt *Payload) {
  auto *LLVMC = toolchain::ConstantFP::getQNaN(Ty->LLVMTy, Negative, Payload);
  return cast<Constant>(Ty->getContext().getOrCreateConstant(LLVMC));
}
Constant *ConstantFP::getSNaN(Type *Ty, bool Negative, APInt *Payload) {
  auto *LLVMC = toolchain::ConstantFP::getSNaN(Ty->LLVMTy, Negative, Payload);
  return cast<Constant>(Ty->getContext().getOrCreateConstant(LLVMC));
}
Constant *ConstantFP::getZero(Type *Ty, bool Negative) {
  auto *LLVMC = toolchain::ConstantFP::getZero(Ty->LLVMTy, Negative);
  return cast<Constant>(Ty->getContext().getOrCreateConstant(LLVMC));
}
Constant *ConstantFP::getNegativeZero(Type *Ty) {
  auto *LLVMC = toolchain::ConstantFP::getNegativeZero(Ty->LLVMTy);
  return cast<Constant>(Ty->getContext().getOrCreateConstant(LLVMC));
}
Constant *ConstantFP::getInfinity(Type *Ty, bool Negative) {
  auto *LLVMC = toolchain::ConstantFP::getInfinity(Ty->LLVMTy, Negative);
  return cast<Constant>(Ty->getContext().getOrCreateConstant(LLVMC));
}
bool ConstantFP::isValueValidForType(Type *Ty, const APFloat &V) {
  return toolchain::ConstantFP::isValueValidForType(Ty->LLVMTy, V);
}

Constant *ConstantArray::get(ArrayType *T, ArrayRef<Constant *> V) {
  auto &Ctx = T->getContext();
  SmallVector<toolchain::Constant *> LLVMValues;
  LLVMValues.reserve(V.size());
  for (auto *Elm : V)
    LLVMValues.push_back(cast<toolchain::Constant>(Elm->Val));
  auto *LLVMC =
      toolchain::ConstantArray::get(cast<toolchain::ArrayType>(T->LLVMTy), LLVMValues);
  return cast<ConstantArray>(Ctx.getOrCreateConstant(LLVMC));
}

ArrayType *ConstantArray::getType() const {
  return cast<ArrayType>(
      Ctx.getType(cast<toolchain::ConstantArray>(Val)->getType()));
}

Constant *ConstantStruct::get(StructType *T, ArrayRef<Constant *> V) {
  auto &Ctx = T->getContext();
  SmallVector<toolchain::Constant *> LLVMValues;
  LLVMValues.reserve(V.size());
  for (auto *Elm : V)
    LLVMValues.push_back(cast<toolchain::Constant>(Elm->Val));
  auto *LLVMC =
      toolchain::ConstantStruct::get(cast<toolchain::StructType>(T->LLVMTy), LLVMValues);
  return cast<ConstantStruct>(Ctx.getOrCreateConstant(LLVMC));
}

StructType *ConstantStruct::getTypeForElements(Context &Ctx,
                                               ArrayRef<Constant *> V,
                                               bool Packed) {
  unsigned VecSize = V.size();
  SmallVector<Type *, 16> EltTypes;
  EltTypes.reserve(VecSize);
  for (Constant *Elm : V)
    EltTypes.push_back(Elm->getType());
  return StructType::get(Ctx, EltTypes, Packed);
}

Constant *ConstantVector::get(ArrayRef<Constant *> V) {
  assert(!V.empty() && "Expected non-empty V!");
  auto &Ctx = V[0]->getContext();
  SmallVector<toolchain::Constant *, 8> LLVMV;
  LLVMV.reserve(V.size());
  for (auto *Elm : V)
    LLVMV.push_back(cast<toolchain::Constant>(Elm->Val));
  return Ctx.getOrCreateConstant(toolchain::ConstantVector::get(LLVMV));
}

Constant *ConstantVector::getSplat(ElementCount EC, Constant *Elt) {
  auto *LLVMElt = cast<toolchain::Constant>(Elt->Val);
  auto &Ctx = Elt->getContext();
  return Ctx.getOrCreateConstant(toolchain::ConstantVector::getSplat(EC, LLVMElt));
}

Constant *ConstantVector::getSplatValue(bool AllowPoison) const {
  auto *LLVMSplatValue = cast_or_null<toolchain::Constant>(
      cast<toolchain::ConstantVector>(Val)->getSplatValue(AllowPoison));
  return LLVMSplatValue ? Ctx.getOrCreateConstant(LLVMSplatValue) : nullptr;
}

ConstantAggregateZero *ConstantAggregateZero::get(Type *Ty) {
  auto *LLVMC = toolchain::ConstantAggregateZero::get(Ty->LLVMTy);
  return cast<ConstantAggregateZero>(
      Ty->getContext().getOrCreateConstant(LLVMC));
}

Constant *ConstantAggregateZero::getSequentialElement() const {
  return cast<Constant>(Ctx.getValue(
      cast<toolchain::ConstantAggregateZero>(Val)->getSequentialElement()));
}
Constant *ConstantAggregateZero::getStructElement(unsigned Elt) const {
  return cast<Constant>(Ctx.getValue(
      cast<toolchain::ConstantAggregateZero>(Val)->getStructElement(Elt)));
}
Constant *ConstantAggregateZero::getElementValue(Constant *C) const {
  return cast<Constant>(
      Ctx.getValue(cast<toolchain::ConstantAggregateZero>(Val)->getElementValue(
          cast<toolchain::Constant>(C->Val))));
}
Constant *ConstantAggregateZero::getElementValue(unsigned Idx) const {
  return cast<Constant>(Ctx.getValue(
      cast<toolchain::ConstantAggregateZero>(Val)->getElementValue(Idx)));
}

ConstantPointerNull *ConstantPointerNull::get(PointerType *Ty) {
  auto *LLVMC =
      toolchain::ConstantPointerNull::get(cast<toolchain::PointerType>(Ty->LLVMTy));
  return cast<ConstantPointerNull>(Ty->getContext().getOrCreateConstant(LLVMC));
}

PointerType *ConstantPointerNull::getType() const {
  return cast<PointerType>(
      Ctx.getType(cast<toolchain::ConstantPointerNull>(Val)->getType()));
}

UndefValue *UndefValue::get(Type *T) {
  auto *LLVMC = toolchain::UndefValue::get(T->LLVMTy);
  return cast<UndefValue>(T->getContext().getOrCreateConstant(LLVMC));
}

UndefValue *UndefValue::getSequentialElement() const {
  return cast<UndefValue>(Ctx.getOrCreateConstant(
      cast<toolchain::UndefValue>(Val)->getSequentialElement()));
}

UndefValue *UndefValue::getStructElement(unsigned Elt) const {
  return cast<UndefValue>(Ctx.getOrCreateConstant(
      cast<toolchain::UndefValue>(Val)->getStructElement(Elt)));
}

UndefValue *UndefValue::getElementValue(Constant *C) const {
  return cast<UndefValue>(
      Ctx.getOrCreateConstant(cast<toolchain::UndefValue>(Val)->getElementValue(
          cast<toolchain::Constant>(C->Val))));
}

UndefValue *UndefValue::getElementValue(unsigned Idx) const {
  return cast<UndefValue>(Ctx.getOrCreateConstant(
      cast<toolchain::UndefValue>(Val)->getElementValue(Idx)));
}

PoisonValue *PoisonValue::get(Type *T) {
  auto *LLVMC = toolchain::PoisonValue::get(T->LLVMTy);
  return cast<PoisonValue>(T->getContext().getOrCreateConstant(LLVMC));
}

PoisonValue *PoisonValue::getSequentialElement() const {
  return cast<PoisonValue>(Ctx.getOrCreateConstant(
      cast<toolchain::PoisonValue>(Val)->getSequentialElement()));
}

PoisonValue *PoisonValue::getStructElement(unsigned Elt) const {
  return cast<PoisonValue>(Ctx.getOrCreateConstant(
      cast<toolchain::PoisonValue>(Val)->getStructElement(Elt)));
}

PoisonValue *PoisonValue::getElementValue(Constant *C) const {
  return cast<PoisonValue>(
      Ctx.getOrCreateConstant(cast<toolchain::PoisonValue>(Val)->getElementValue(
          cast<toolchain::Constant>(C->Val))));
}

PoisonValue *PoisonValue::getElementValue(unsigned Idx) const {
  return cast<PoisonValue>(Ctx.getOrCreateConstant(
      cast<toolchain::PoisonValue>(Val)->getElementValue(Idx)));
}

void GlobalVariable::setAlignment(MaybeAlign Align) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&GlobalVariable::getAlign,
                                       &GlobalVariable::setAlignment>>(this);
  cast<toolchain::GlobalVariable>(Val)->setAlignment(Align);
}

void GlobalObject::setSection(StringRef S) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&GlobalObject::getSection, &GlobalObject::setSection>>(
          this);
  cast<toolchain::GlobalObject>(Val)->setSection(S);
}

template <typename GlobalT, typename LLVMGlobalT, typename ParentT,
          typename LLVMParentT>
GlobalT &GlobalWithNodeAPI<GlobalT, LLVMGlobalT, ParentT, LLVMParentT>::
    LLVMGVToGV::operator()(LLVMGlobalT &LLVMGV) const {
  return cast<GlobalT>(*Ctx.getValue(&LLVMGV));
}

// Explicit instantiations.
template class LLVM_EXPORT_TEMPLATE GlobalWithNodeAPI<
    GlobalIFunc, toolchain::GlobalIFunc, GlobalObject, toolchain::GlobalObject>;
template class LLVM_EXPORT_TEMPLATE GlobalWithNodeAPI<
    Function, toolchain::Function, GlobalObject, toolchain::GlobalObject>;
template class LLVM_EXPORT_TEMPLATE GlobalWithNodeAPI<
    GlobalVariable, toolchain::GlobalVariable, GlobalObject, toolchain::GlobalObject>;
template class LLVM_EXPORT_TEMPLATE GlobalWithNodeAPI<
    GlobalAlias, toolchain::GlobalAlias, GlobalValue, toolchain::GlobalValue>;

void GlobalIFunc::setResolver(Constant *Resolver) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&GlobalIFunc::getResolver, &GlobalIFunc::setResolver>>(
          this);
  cast<toolchain::GlobalIFunc>(Val)->setResolver(
      cast<toolchain::Constant>(Resolver->Val));
}

Constant *GlobalIFunc::getResolver() const {
  return Ctx.getOrCreateConstant(cast<toolchain::GlobalIFunc>(Val)->getResolver());
}

Function *GlobalIFunc::getResolverFunction() {
  return cast<Function>(Ctx.getOrCreateConstant(
      cast<toolchain::GlobalIFunc>(Val)->getResolverFunction()));
}

GlobalVariable &
GlobalVariable::LLVMGVToGV::operator()(toolchain::GlobalVariable &LLVMGV) const {
  return cast<GlobalVariable>(*Ctx.getValue(&LLVMGV));
}

Constant *GlobalVariable::getInitializer() const {
  return Ctx.getOrCreateConstant(
      cast<toolchain::GlobalVariable>(Val)->getInitializer());
}

void GlobalVariable::setInitializer(Constant *InitVal) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&GlobalVariable::getInitializer,
                                       &GlobalVariable::setInitializer>>(this);
  cast<toolchain::GlobalVariable>(Val)->setInitializer(
      cast<toolchain::Constant>(InitVal->Val));
}

void GlobalVariable::setConstant(bool V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&GlobalVariable::isConstant,
                                       &GlobalVariable::setConstant>>(this);
  cast<toolchain::GlobalVariable>(Val)->setConstant(V);
}

void GlobalVariable::setExternallyInitialized(bool V) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&GlobalVariable::isExternallyInitialized,
                        &GlobalVariable::setExternallyInitialized>>(this);
  cast<toolchain::GlobalVariable>(Val)->setExternallyInitialized(V);
}

void GlobalAlias::setAliasee(Constant *Aliasee) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&GlobalAlias::getAliasee, &GlobalAlias::setAliasee>>(
          this);
  cast<toolchain::GlobalAlias>(Val)->setAliasee(cast<toolchain::Constant>(Aliasee->Val));
}

Constant *GlobalAlias::getAliasee() const {
  return cast<Constant>(
      Ctx.getOrCreateConstant(cast<toolchain::GlobalAlias>(Val)->getAliasee()));
}

const GlobalObject *GlobalAlias::getAliaseeObject() const {
  return cast<GlobalObject>(Ctx.getOrCreateConstant(
      cast<toolchain::GlobalAlias>(Val)->getAliaseeObject()));
}

void GlobalValue::setUnnamedAddr(UnnamedAddr V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&GlobalValue::getUnnamedAddr,
                                       &GlobalValue::setUnnamedAddr>>(this);
  cast<toolchain::GlobalValue>(Val)->setUnnamedAddr(V);
}

void GlobalValue::setVisibility(VisibilityTypes V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&GlobalValue::getVisibility,
                                       &GlobalValue::setVisibility>>(this);
  cast<toolchain::GlobalValue>(Val)->setVisibility(V);
}

NoCFIValue *NoCFIValue::get(GlobalValue *GV) {
  auto *LLVMC = toolchain::NoCFIValue::get(cast<toolchain::GlobalValue>(GV->Val));
  return cast<NoCFIValue>(GV->getContext().getOrCreateConstant(LLVMC));
}

GlobalValue *NoCFIValue::getGlobalValue() const {
  auto *LLVMC = cast<toolchain::NoCFIValue>(Val)->getGlobalValue();
  return cast<GlobalValue>(Ctx.getOrCreateConstant(LLVMC));
}

PointerType *NoCFIValue::getType() const {
  return cast<PointerType>(Ctx.getType(cast<toolchain::NoCFIValue>(Val)->getType()));
}

ConstantPtrAuth *ConstantPtrAuth::get(Constant *Ptr, ConstantInt *Key,
                                      ConstantInt *Disc, Constant *AddrDisc,
                                      Constant *DeactivationSymbol) {
  auto *LLVMC = toolchain::ConstantPtrAuth::get(
      cast<toolchain::Constant>(Ptr->Val), cast<toolchain::ConstantInt>(Key->Val),
      cast<toolchain::ConstantInt>(Disc->Val), cast<toolchain::Constant>(AddrDisc->Val),
      cast<toolchain::Constant>(DeactivationSymbol->Val));
  return cast<ConstantPtrAuth>(Ptr->getContext().getOrCreateConstant(LLVMC));
}

Constant *ConstantPtrAuth::getPointer() const {
  return Ctx.getOrCreateConstant(
      cast<toolchain::ConstantPtrAuth>(Val)->getPointer());
}

ConstantInt *ConstantPtrAuth::getKey() const {
  return cast<ConstantInt>(
      Ctx.getOrCreateConstant(cast<toolchain::ConstantPtrAuth>(Val)->getKey()));
}

ConstantInt *ConstantPtrAuth::getDiscriminator() const {
  return cast<ConstantInt>(Ctx.getOrCreateConstant(
      cast<toolchain::ConstantPtrAuth>(Val)->getDiscriminator()));
}

Constant *ConstantPtrAuth::getAddrDiscriminator() const {
  return Ctx.getOrCreateConstant(
      cast<toolchain::ConstantPtrAuth>(Val)->getAddrDiscriminator());
}

Constant *ConstantPtrAuth::getDeactivationSymbol() const {
  return Ctx.getOrCreateConstant(
      cast<toolchain::ConstantPtrAuth>(Val)->getDeactivationSymbol());
}

ConstantPtrAuth *ConstantPtrAuth::getWithSameSchema(Constant *Pointer) const {
  auto *LLVMC = cast<toolchain::ConstantPtrAuth>(Val)->getWithSameSchema(
      cast<toolchain::Constant>(Pointer->Val));
  return cast<ConstantPtrAuth>(Ctx.getOrCreateConstant(LLVMC));
}

BlockAddress *BlockAddress::get(Function *F, BasicBlock *BB) {
  auto *LLVMC = toolchain::BlockAddress::get(cast<toolchain::Function>(F->Val),
                                        cast<toolchain::BasicBlock>(BB->Val));
  return cast<BlockAddress>(F->getContext().getOrCreateConstant(LLVMC));
}

BlockAddress *BlockAddress::get(BasicBlock *BB) {
  auto *LLVMC = toolchain::BlockAddress::get(cast<toolchain::BasicBlock>(BB->Val));
  return cast<BlockAddress>(BB->getContext().getOrCreateConstant(LLVMC));
}

BlockAddress *BlockAddress::lookup(const BasicBlock *BB) {
  auto *LLVMC = toolchain::BlockAddress::lookup(cast<toolchain::BasicBlock>(BB->Val));
  return cast_or_null<BlockAddress>(BB->getContext().getValue(LLVMC));
}

Function *BlockAddress::getFunction() const {
  return cast<Function>(
      Ctx.getValue(cast<toolchain::BlockAddress>(Val)->getFunction()));
}

BasicBlock *BlockAddress::getBasicBlock() const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::BlockAddress>(Val)->getBasicBlock()));
}

DSOLocalEquivalent *DSOLocalEquivalent::get(GlobalValue *GV) {
  auto *LLVMC = toolchain::DSOLocalEquivalent::get(cast<toolchain::GlobalValue>(GV->Val));
  return cast<DSOLocalEquivalent>(GV->getContext().getValue(LLVMC));
}

GlobalValue *DSOLocalEquivalent::getGlobalValue() const {
  return cast<GlobalValue>(
      Ctx.getValue(cast<toolchain::DSOLocalEquivalent>(Val)->getGlobalValue()));
}

} // namespace vm::core::sandboxir
