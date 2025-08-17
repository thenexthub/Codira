//===--- ConstantInitBuilder.cpp - Global initializer builder -------------===//
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
//
// This file defines out-of-line routines for building initializers for
// global variables, in particular the kind of globals that are implicitly
// introduced by various language ABIs.
//
//===----------------------------------------------------------------------===//

#include "language/Core/CodeGen/ConstantInitBuilder.h"
#include "CodeGenModule.h"

using namespace language::Core;
using namespace CodeGen;

toolchain::Type *ConstantInitFuture::getType() const {
  assert(Data && "dereferencing null future");
  if (const auto *C = dyn_cast<toolchain::Constant *>(Data)) {
    return C->getType();
  } else {
    return cast<ConstantInitBuilderBase *>(Data)->Buffer[0]->getType();
  }
}

void ConstantInitFuture::abandon() {
  assert(Data && "abandoning null future");
  if (auto *builder = dyn_cast<ConstantInitBuilderBase *>(Data)) {
    builder->abandon(0);
  }
  Data = nullptr;
}

void ConstantInitFuture::installInGlobal(toolchain::GlobalVariable *GV) {
  assert(Data && "installing null future");
  if (auto *C = dyn_cast<toolchain::Constant *>(Data)) {
    GV->setInitializer(C);
  } else {
    auto &builder = *cast<ConstantInitBuilderBase *>(Data);
    assert(builder.Buffer.size() == 1);
    builder.setGlobalInitializer(GV, builder.Buffer[0]);
    builder.Buffer.clear();
    Data = nullptr;
  }
}

ConstantInitFuture
ConstantInitBuilderBase::createFuture(toolchain::Constant *initializer) {
  assert(Buffer.empty() && "buffer not current empty");
  Buffer.push_back(initializer);
  return ConstantInitFuture(this);
}

// Only used in this file.
inline ConstantInitFuture::ConstantInitFuture(ConstantInitBuilderBase *builder)
    : Data(builder) {
  assert(!builder->Frozen);
  assert(builder->Buffer.size() == 1);
  assert(builder->Buffer[0] != nullptr);
}

toolchain::GlobalVariable *
ConstantInitBuilderBase::createGlobal(toolchain::Constant *initializer,
                                      const toolchain::Twine &name,
                                      CharUnits alignment,
                                      bool constant,
                                      toolchain::GlobalValue::LinkageTypes linkage,
                                      unsigned addressSpace) {
  auto GV = new toolchain::GlobalVariable(CGM.getModule(),
                                     initializer->getType(),
                                     constant,
                                     linkage,
                                     initializer,
                                     name,
                                     /*insert before*/ nullptr,
                                     toolchain::GlobalValue::NotThreadLocal,
                                     addressSpace);
  GV->setAlignment(alignment.getAsAlign());
  resolveSelfReferences(GV);
  return GV;
}

void ConstantInitBuilderBase::setGlobalInitializer(toolchain::GlobalVariable *GV,
                                                   toolchain::Constant *initializer){
  GV->setInitializer(initializer);

  if (!SelfReferences.empty())
    resolveSelfReferences(GV);
}

void ConstantInitBuilderBase::resolveSelfReferences(toolchain::GlobalVariable *GV) {
  for (auto &entry : SelfReferences) {
    toolchain::Constant *resolvedReference =
      toolchain::ConstantExpr::getInBoundsGetElementPtr(
        GV->getValueType(), GV, entry.Indices);
    auto dummy = entry.Dummy;
    dummy->replaceAllUsesWith(resolvedReference);
    dummy->eraseFromParent();
  }
  SelfReferences.clear();
}

void ConstantInitBuilderBase::abandon(size_t newEnd) {
  // Remove all the entries we've added.
  Buffer.erase(Buffer.begin() + newEnd, Buffer.end());

  // If we're abandoning all the way to the beginning, destroy
  // all the self-references, because we might not get another
  // opportunity.
  if (newEnd == 0) {
    for (auto &entry : SelfReferences) {
      auto dummy = entry.Dummy;
      dummy->replaceAllUsesWith(toolchain::PoisonValue::get(dummy->getType()));
      dummy->eraseFromParent();
    }
    SelfReferences.clear();
  }
}

void ConstantAggregateBuilderBase::addSize(CharUnits size) {
  add(Builder.CGM.getSize(size));
}

toolchain::Constant *
ConstantAggregateBuilderBase::getRelativeOffset(toolchain::IntegerType *offsetType,
                                                toolchain::Constant *target) {
  return getRelativeOffsetToPosition(offsetType, target,
                                     Builder.Buffer.size() - Begin);
}

toolchain::Constant *ConstantAggregateBuilderBase::getRelativeOffsetToPosition(
    toolchain::IntegerType *offsetType, toolchain::Constant *target, size_t position) {
  // Compute the address of the relative-address slot.
  auto base = getAddrOfPosition(offsetType, position);

  // Subtract.
  base = toolchain::ConstantExpr::getPtrToInt(base, Builder.CGM.IntPtrTy);
  target = toolchain::ConstantExpr::getPtrToInt(target, Builder.CGM.IntPtrTy);
  toolchain::Constant *offset = toolchain::ConstantExpr::getSub(target, base);

  // Truncate to the relative-address type if necessary.
  if (Builder.CGM.IntPtrTy != offsetType) {
    offset = toolchain::ConstantExpr::getTrunc(offset, offsetType);
  }

  return offset;
}

toolchain::Constant *
ConstantAggregateBuilderBase::getAddrOfPosition(toolchain::Type *type,
                                                size_t position) {
  // Make a global variable.  We will replace this with a GEP to this
  // position after installing the initializer.
  auto dummy = new toolchain::GlobalVariable(Builder.CGM.getModule(), type, true,
                                        toolchain::GlobalVariable::PrivateLinkage,
                                        nullptr, "");
  Builder.SelfReferences.emplace_back(dummy);
  auto &entry = Builder.SelfReferences.back();
  getGEPIndicesTo(entry.Indices, position + Begin);
  return dummy;
}

toolchain::Constant *
ConstantAggregateBuilderBase::getAddrOfCurrentPosition(toolchain::Type *type) {
  // Make a global variable.  We will replace this with a GEP to this
  // position after installing the initializer.
  auto dummy =
    new toolchain::GlobalVariable(Builder.CGM.getModule(), type, true,
                             toolchain::GlobalVariable::PrivateLinkage,
                             nullptr, "");
  Builder.SelfReferences.emplace_back(dummy);
  auto &entry = Builder.SelfReferences.back();
  (void) getGEPIndicesToCurrentPosition(entry.Indices);
  return dummy;
}

void ConstantAggregateBuilderBase::getGEPIndicesTo(
                               toolchain::SmallVectorImpl<toolchain::Constant*> &indices,
                               size_t position) const {
  // Recurse on the parent builder if present.
  if (Parent) {
    Parent->getGEPIndicesTo(indices, Begin);

  // Otherwise, add an index to drill into the first level of pointer.
  } else {
    assert(indices.empty());
    indices.push_back(toolchain::ConstantInt::get(Builder.CGM.Int32Ty, 0));
  }

  assert(position >= Begin);
  // We have to use i32 here because struct GEPs demand i32 indices.
  // It's rather unlikely to matter in practice.
  indices.push_back(toolchain::ConstantInt::get(Builder.CGM.Int32Ty,
                                           position - Begin));
}

ConstantAggregateBuilderBase::PlaceholderPosition
ConstantAggregateBuilderBase::addPlaceholderWithSize(toolchain::Type *type) {
  // Bring the offset up to the last field.
  CharUnits offset = getNextOffsetFromGlobal();

  // Create the placeholder.
  auto position = addPlaceholder();

  // Advance the offset past that field.
  auto &layout = Builder.CGM.getDataLayout();
  if (!Packed)
    offset = offset.alignTo(CharUnits::fromQuantity(layout.getABITypeAlign(type)));
  offset += CharUnits::fromQuantity(layout.getTypeStoreSize(type));

  CachedOffsetEnd = Builder.Buffer.size();
  CachedOffsetFromGlobal = offset;

  return position;
}

CharUnits ConstantAggregateBuilderBase::getOffsetFromGlobalTo(size_t end) const{
  size_t cacheEnd = CachedOffsetEnd;
  assert(cacheEnd <= end);

  // Fast path: if the cache is valid, just use it.
  if (cacheEnd == end) {
    return CachedOffsetFromGlobal;
  }

  // If the cached range ends before the index at which the current
  // aggregate starts, recurse for the parent.
  CharUnits offset;
  if (cacheEnd < Begin) {
    assert(cacheEnd == 0);
    assert(Parent && "Begin != 0 for root builder");
    cacheEnd = Begin;
    offset = Parent->getOffsetFromGlobalTo(Begin);
  } else {
    offset = CachedOffsetFromGlobal;
  }

  // Perform simple layout on the elements in cacheEnd..<end.
  if (cacheEnd != end) {
    auto &layout = Builder.CGM.getDataLayout();
    do {
      toolchain::Constant *element = Builder.Buffer[cacheEnd];
      assert(element != nullptr &&
             "cannot compute offset when a placeholder is present");
      toolchain::Type *elementType = element->getType();
      if (!Packed)
        offset = offset.alignTo(
            CharUnits::fromQuantity(layout.getABITypeAlign(elementType)));
      offset += CharUnits::fromQuantity(layout.getTypeStoreSize(elementType));
    } while (++cacheEnd != end);
  }

  // Cache and return.
  CachedOffsetEnd = cacheEnd;
  CachedOffsetFromGlobal = offset;
  return offset;
}

toolchain::Constant *ConstantAggregateBuilderBase::finishArray(toolchain::Type *eltTy) {
  markFinished();

  auto &buffer = getBuffer();
  assert((Begin < buffer.size() ||
          (Begin == buffer.size() && eltTy))
         && "didn't add any array elements without element type");
  auto elts = toolchain::ArrayRef(buffer).slice(Begin);
  if (!eltTy) eltTy = elts[0]->getType();
  auto type = toolchain::ArrayType::get(eltTy, elts.size());
  auto constant = toolchain::ConstantArray::get(type, elts);
  buffer.erase(buffer.begin() + Begin, buffer.end());
  return constant;
}

toolchain::Constant *
ConstantAggregateBuilderBase::finishStruct(toolchain::StructType *ty) {
  markFinished();

  auto &buffer = getBuffer();
  auto elts = toolchain::ArrayRef(buffer).slice(Begin);

  if (ty == nullptr && elts.empty())
    ty = toolchain::StructType::get(Builder.CGM.getLLVMContext(), {}, Packed);

  toolchain::Constant *constant;
  if (ty) {
    assert(ty->isPacked() == Packed);
    constant = toolchain::ConstantStruct::get(ty, elts);
  } else {
    constant = toolchain::ConstantStruct::getAnon(elts, Packed);
  }

  buffer.erase(buffer.begin() + Begin, buffer.end());
  return constant;
}

/// Sign the given pointer and add it to the constant initializer
/// currently being built.
void ConstantAggregateBuilderBase::addSignedPointer(
    toolchain::Constant *Pointer, const PointerAuthSchema &Schema,
    GlobalDecl CalleeDecl, QualType CalleeType) {
  if (!Schema || !Builder.CGM.shouldSignPointer(Schema))
    return add(Pointer);

  toolchain::Constant *StorageAddress = nullptr;
  if (Schema.isAddressDiscriminated()) {
    StorageAddress = getAddrOfCurrentPosition(Pointer->getType());
  }

  toolchain::Constant *SignedPointer = Builder.CGM.getConstantSignedPointer(
      Pointer, Schema, StorageAddress, CalleeDecl, CalleeType);
  add(SignedPointer);
}
