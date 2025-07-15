//===--- GenOpaque.h - Codira IR generation for opaque values ----*- C++ -*-===//
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
//  This file provides a private interface for interacting with opaque
//  values and their value witnesses.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENOPAQUE_H
#define LANGUAGE_IRGEN_GENOPAQUE_H

namespace toolchain {
  class Type;
  class Value;
}

namespace language {
namespace irgen {
  class Address;
  class IRGenFunction;
  class IRGenModule;
  class TypeInfo;
  enum class ValueWitness : unsigned;
  class WitnessIndex;

  /// Return the size of a fixed buffer.
  Size getFixedBufferSize(IRGenModule &IGM);

  /// Return the alignment of a fixed buffer.
  Alignment getFixedBufferAlignment(IRGenModule &IGM);

  /// Given a witness table (protocol or value), return the address of the slot
  /// for one of the witnesses.
  /// If \p areEntriesRelative is true we are emitting code for a relative
  /// protocol witness table.
  Address slotForLoadOfOpaqueWitness(IRGenFunction &IGF, toolchain::Value *table,
                                     WitnessIndex index,
                                     bool areEntriesRelative = false);

  /// Given a witness table (protocol or value), load one of the
  /// witnesses.
  ///
  /// The load is marked invariant. This should not be used in contexts where
  /// the referenced witness table is still undergoing initialization.
  toolchain::Value *emitInvariantLoadOfOpaqueWitness(IRGenFunction &IGF,
                                                bool isProtocolWitness,
                                                toolchain::Value *table,
                                                WitnessIndex index,
                                                toolchain::Value **slot = nullptr);

  /// Given a witness table (protocol or value), load one of the
  /// witnesses.
  ///
  /// The load is marked invariant. This should not be used in contexts where
  /// the referenced witness table is still undergoing initialization.
  toolchain::Value *emitInvariantLoadOfOpaqueWitness(IRGenFunction &IGF,
                                                bool isProtocolWitness,
                                                toolchain::Value *table,
                                                toolchain::Value *index,
                                                toolchain::Value **slot = nullptr);

  /// Emit a call to do an 'initializeBufferWithCopyOfBuffer' operation.
  toolchain::Value *emitInitializeBufferWithCopyOfBufferCall(IRGenFunction &IGF,
                                                        toolchain::Value *metadata,
                                                        Address destBuffer,
                                                        Address srcBuffer);

  /// Emit a call to do an 'initializeBufferWithCopyOfBuffer' operation.
  toolchain::Value *emitInitializeBufferWithCopyOfBufferCall(IRGenFunction &IGF,
                                                        SILType T,
                                                        Address destBuffer,
                                                        Address srcBuffer);

  /// Emit a call to do an 'initializeWithCopy' operation.
  void emitInitializeWithCopyCall(IRGenFunction &IGF,
                                  SILType T,
                                  Address destObject,
                                  Address srcObject);
  toolchain::Value *emitInitializeWithCopyCall(IRGenFunction &IGF,
                                          toolchain::Value *metadata, Address dest,
                                          Address src);

  /// Emit a call to do an 'initializeArrayWithCopy' operation.
  void emitInitializeArrayWithCopyCall(IRGenFunction &IGF,
                                       SILType T,
                                       Address destObject,
                                       Address srcObject,
                                       toolchain::Value *count);

  /// Emit a call to do an 'initializeWithTake' operation.
  void emitInitializeWithTakeCall(IRGenFunction &IGF,
                                  SILType T,
                                  Address destObject,
                                  Address srcObject);
  toolchain::Value *emitInitializeWithTakeCall(IRGenFunction &IGF,
                                          toolchain::Value *metadata, Address dest,
                                          Address src);

  /// Emit a call to do an 'initializeArrayWithTakeNoAlias' operation.
  void emitInitializeArrayWithTakeNoAliasCall(IRGenFunction &IGF, SILType T,
                                              Address destObject,
                                              Address srcObject,
                                              toolchain::Value *count);

  /// Emit a call to do an 'initializeArrayWithTakeFrontToBack' operation.
  void emitInitializeArrayWithTakeFrontToBackCall(IRGenFunction &IGF,
                                                  SILType T,
                                                  Address destObject,
                                                  Address srcObject,
                                                  toolchain::Value *count);

  /// Emit a call to do an 'initializeArrayWithTakeBackToFront' operation.
  void emitInitializeArrayWithTakeBackToFrontCall(IRGenFunction &IGF,
                                                  SILType T,
                                                  Address destObject,
                                                  Address srcObject,
                                                  toolchain::Value *count);

  /// Emit a call to do an 'assignWithCopy' operation.
  void emitAssignWithCopyCall(IRGenFunction &IGF,
                              SILType T,
                              Address destObject,
                              Address srcObject);
  void emitAssignWithCopyCall(IRGenFunction &IGF,
                              toolchain::Value *metadata,
                              Address destObject,
                              Address srcObject);

  /// Emit a call to do an 'assignArrayWithCopyNoAlias' operation.
  void emitAssignArrayWithCopyNoAliasCall(IRGenFunction &IGF, SILType T,
                                          Address destObject, Address srcObject,
                                          toolchain::Value *count);

  /// Emit a call to do an 'assignArrayWithCopyFrontToBack' operation.
  void emitAssignArrayWithCopyFrontToBackCall(IRGenFunction &IGF, SILType T,
                                              Address destObject,
                                              Address srcObject,
                                              toolchain::Value *count);

  /// Emit a call to do an 'assignArrayWithCopyBackToFront' operation.
  void emitAssignArrayWithCopyBackToFrontCall(IRGenFunction &IGF, SILType T,
                                              Address destObject,
                                              Address srcObject,
                                              toolchain::Value *count);

  /// Emit a call to do an 'assignWithTake' operation.
  void emitAssignWithTakeCall(IRGenFunction &IGF,
                              SILType T,
                              Address destObject,
                              Address srcObject);

  /// Emit a call to do an 'assignArrayWithTake' operation.
  void emitAssignArrayWithTakeCall(IRGenFunction &IGF, SILType T,
                                   Address destObject, Address srcObject,
                                   toolchain::Value *count);

  /// Emit a call to do a 'destroy' operation.
  void emitDestroyCall(IRGenFunction &IGF,
                       SILType T,
                       Address object);

  void emitDestroyCall(IRGenFunction &IGF, toolchain::Value *metadata,
                       Address object);

  /// Emit a call to do a 'destroyArray' operation.
  void emitDestroyArrayCall(IRGenFunction &IGF,
                            SILType T,
                            Address object,
                            toolchain::Value *count);

  /// Emit a call to the 'getEnumTagSinglePayload' operation.
  toolchain::Value *emitGetEnumTagSinglePayloadCall(IRGenFunction &IGF, SILType T,
                                               toolchain::Value *numEmptyCases,
                                               Address destObject);

  /// Emit a call to the 'storeEnumTagSinglePayload' operation.
  void emitStoreEnumTagSinglePayloadCall(IRGenFunction &IGF, SILType T,
                                         toolchain::Value *whichCase,
                                         toolchain::Value *numEmptyCases,
                                         Address destObject);

  /// Emit a call to the 'getEnumTag' operation.
  toolchain::Value *emitGetEnumTagCall(IRGenFunction &IGF,
                                  SILType T,
                                  Address srcObject);

  /// Emit a call to the 'destructiveProjectEnumData' operation.
  /// The type must be dynamically known to have enum witnesses.
  void emitDestructiveProjectEnumDataCall(IRGenFunction &IGF,
                                          SILType T,
                                          Address srcObject);

  /// Emit a call to the 'destructiveInjectEnumTag' operation.
  /// The type must be dynamically known to have enum witnesses.
  void emitDestructiveInjectEnumTagCall(IRGenFunction &IGF,
                                        SILType T,
                                        toolchain::Value *tag,
                                        Address srcObject);

  /// Emit a load of the 'size' value witness.
  toolchain::Value *emitLoadOfSize(IRGenFunction &IGF, SILType T);

  /// Emit a load of the 'stride' value witness.
  toolchain::Value *emitLoadOfStride(IRGenFunction &IGF, SILType T);

  /// Emit a load of the 'alignmentMask' value witness.
  toolchain::Value *emitLoadOfAlignmentMask(IRGenFunction &IGF, SILType T);

  /// Emit a load of the 'isTriviallyDestroyable' value witness.
  toolchain::Value *emitLoadOfIsTriviallyDestroyable(IRGenFunction &IGF, SILType T);

  /// Emit a load of the 'isBitwiseTakable' value witness.
  toolchain::Value *emitLoadOfIsBitwiseTakable(IRGenFunction &IGF, SILType T);

  /// Emit a load of the 'isInline' value witness.
  toolchain::Value *emitLoadOfIsInline(IRGenFunction &IGF, SILType T);

  /// Emit a load of the 'extraInhabitantCount' value witness.
  toolchain::Value *emitLoadOfExtraInhabitantCount(IRGenFunction &IGF, SILType T);

  /// Emit a stored to the 'extraInhabitantCount' value witness.
  void emitStoreOfExtraInhabitantCount(IRGenFunction &IGF, toolchain::Value *val,
                                       toolchain::Value *metadata);

  /// Returns the IsInline flag and the loaded flags value.
  std::pair<toolchain::Value *, toolchain::Value *>
  emitLoadOfIsInline(IRGenFunction &IGF, toolchain::Value *metadata);

  /// Emits the alignment mask value from a loaded flags value.
  toolchain::Value *emitAlignMaskFromFlags(IRGenFunction &IGF, toolchain::Value *flags);

  toolchain::Value *emitLoadOfSize(IRGenFunction &IGF, toolchain::Value *metadata);

  /// Allocate/project/allocate memory for a value of the type in the fixed size
  /// buffer.
  Address emitAllocateValueInBuffer(IRGenFunction &IGF,
                               SILType type,
                               Address buffer);
  Address emitProjectValueInBuffer(IRGenFunction &IGF,
                              SILType type,
                              Address buffer);

  using GetExtraInhabitantTagEmitter =
    toolchain::function_ref<toolchain::Value*(IRGenFunction &IGF,
                                    Address addr,
                                    toolchain::Value *xiCount)>;

  toolchain::Constant *
  getOrCreateGetExtraInhabitantTagFunction(IRGenModule &IGM,
                                           SILType objectType,
                                           const TypeInfo &objectTI,
                                           GetExtraInhabitantTagEmitter emit);

  toolchain::Value *
  emitGetEnumTagSinglePayloadGenericCall(IRGenFunction &IGF,
                                         SILType payloadType,
                                         const TypeInfo &payloadTI,
                                         toolchain::Value *numExtraCases,
                                         Address address,
                                         GetExtraInhabitantTagEmitter emit);

  using StoreExtraInhabitantTagEmitter =
    toolchain::function_ref<void(IRGenFunction &IGF,
                            Address addr,
                            toolchain::Value *tag,
                            toolchain::Value *xiCount)>;

  toolchain::Constant *
  getOrCreateStoreExtraInhabitantTagFunction(IRGenModule &IGM,
                                             SILType objectType,
                                             const TypeInfo &objectTI,
                                        StoreExtraInhabitantTagEmitter emit);

  void emitStoreEnumTagSinglePayloadGenericCall(IRGenFunction &IGF,
                                                SILType payloadType,
                                                const TypeInfo &payloadTI,
                                                toolchain::Value *index,
                                                toolchain::Value *numExtraCases,
                                                Address address,
                                           StoreExtraInhabitantTagEmitter emit);
} // end namespace irgen
} // end namespace language

#endif
