//===--- ResilientTypeInfo.h - Resilient-layout types -----------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file defines a class used for implementing non-class-bound
//  archetypes, and resilient structs and enums. Values of these types are
//  opaque and must be manipulated through value witness function calls.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_RESILIENTTYPEINFO_H
#define LANGUAGE_IRGEN_RESILIENTTYPEINFO_H

#include "NonFixedTypeInfo.h"
#include "Outlining.h"

namespace language {
namespace irgen {

/// An abstract CRTP class designed for types whose values are manipulated
/// indirectly through value witness functions.
///
/// We build upon WitnessSizedTypeInfo, adding the additional structure
/// that the opaque value has the same size as the underlying type with
/// no additional metadata, distinguishing this case from other uses of
/// WitnessSizedTypeInfo, which are existentials (these add conformance
/// tables) and fragile enums with generic payloads (these add tag bits).
///
/// This allows us to make use of array value witness functions, and
/// more importantly, to forward extra inhabitant information from the
/// concrete type. This ensures that enums have the correct layout
/// when accessed at different abstraction levels or from different
/// resilience scopes.
///
template <class Impl>
class ResilientTypeInfo : public WitnessSizedTypeInfo<Impl> {
protected:
  ResilientTypeInfo(toolchain::Type *type,
                    IsCopyable_t copyable,
                    IsABIAccessible_t abiAccessible)
    : WitnessSizedTypeInfo<Impl>(type, Alignment(1),
                                 IsNotTriviallyDestroyable, IsNotBitwiseTakable,
                                 copyable,
                                 abiAccessible) {}

public:
  void assignWithCopy(IRGenFunction &IGF, Address dest, Address src, SILType T,
                      bool isOutlined) const override {
    emitAssignWithCopyCall(IGF, T, dest, src);
  }

  void assignArrayWithCopyNoAlias(IRGenFunction &IGF, Address dest, Address src,
                                  toolchain::Value *count,
                                  SILType T) const override {
    emitAssignArrayWithCopyNoAliasCall(IGF, T, dest, src, count);
  }

  void assignArrayWithCopyFrontToBack(IRGenFunction &IGF, Address dest,
                                      Address src, toolchain::Value *count,
                                      SILType T) const override {
    emitAssignArrayWithCopyFrontToBackCall(IGF, T, dest, src, count);
  }

  void assignArrayWithCopyBackToFront(IRGenFunction &IGF, Address dest,
                                      Address src, toolchain::Value *count,
                                      SILType T) const override {
    emitAssignArrayWithCopyBackToFrontCall(IGF, T, dest, src, count);
  }

  void assignWithTake(IRGenFunction &IGF, Address dest, Address src, SILType T,
                      bool isOutlined) const override {
    emitAssignWithTakeCall(IGF, T, dest, src);
  }

  void assignArrayWithTake(IRGenFunction &IGF, Address dest, Address src,
                           toolchain::Value *count, SILType T) const override {
    emitAssignArrayWithTakeCall(IGF, T, dest, src, count);
  }

  Address initializeBufferWithCopyOfBuffer(IRGenFunction &IGF,
                                   Address dest, Address src,
                                   SILType T) const override {
    auto addr = emitInitializeBufferWithCopyOfBufferCall(IGF, T, dest, src);
    return this->getAddressForPointer(addr);
  }

  void initializeWithCopy(IRGenFunction &IGF, Address dest, Address src,
                          SILType T, bool isOutlined) const override {
    emitInitializeWithCopyCall(IGF, T, dest, src);
  }

  void initializeArrayWithCopy(IRGenFunction &IGF,
                               Address dest, Address src, toolchain::Value *count,
                               SILType T) const override {
    emitInitializeArrayWithCopyCall(IGF, T, dest, src, count);
  }

  void initializeWithTake(IRGenFunction &IGF, Address dest, Address src,
                          SILType T, bool isOutlined,
                          bool zeroizeIfSensitive) const override {
    emitInitializeWithTakeCall(IGF, T, dest, src);
  }

  void initializeArrayWithTakeNoAlias(IRGenFunction &IGF, Address dest,
                                      Address src, toolchain::Value *count,
                                      SILType T) const override {
    emitInitializeArrayWithTakeNoAliasCall(IGF, T, dest, src, count);
  }

  void initializeArrayWithTakeFrontToBack(IRGenFunction &IGF,
                                          Address dest, Address src,
                                          toolchain::Value *count,
                                          SILType T) const override {
    emitInitializeArrayWithTakeFrontToBackCall(IGF, T, dest, src, count);
  }

  void initializeArrayWithTakeBackToFront(IRGenFunction &IGF,
                                          Address dest, Address src,
                                          toolchain::Value *count,
                                          SILType T) const override {
    emitInitializeArrayWithTakeBackToFrontCall(IGF, T, dest, src, count);
  }

  void destroy(IRGenFunction &IGF, Address addr, SILType T,
               bool isOutlined) const override {
    emitDestroyCall(IGF, T, addr);
  }

  void destroyArray(IRGenFunction &IGF, Address addr, toolchain::Value *count,
                    SILType T) const override {
    emitDestroyArrayCall(IGF, T, addr, count);
  }

  bool mayHaveExtraInhabitants(IRGenModule &IGM) const override {
    return true;
  }

  toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                       toolchain::Value *numEmptyCases,
                                       Address enumAddr,
                                       SILType T,
                                       bool isOutlined) const override {
    return emitGetEnumTagSinglePayloadCall(IGF, T, numEmptyCases, enumAddr);
  }

  void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *whichCase,
                                 toolchain::Value *numEmptyCases, Address enumAddr,
                                 SILType T, bool isOutlined) const override {
    emitStoreEnumTagSinglePayloadCall(IGF, T, whichCase, numEmptyCases, enumAddr);
  }

  void collectMetadataForOutlining(OutliningMetadataCollector &collector,
                                   SILType T) const override {
    collector.collectTypeMetadata(T);
  }
};

}
}

#endif
