//===--- GenProto.h - Codira IR generation for prototypes --------*- C++ -*-===//
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
//  This file provides the private interface to the protocol-emission code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENPROTO_H
#define LANGUAGE_IRGEN_GENPROTO_H

#include "language/SIL/SILFunction.h"

#include "Fulfillment.h"
#include "GenericRequirement.h"
#include "MetadataSource.h"

namespace toolchain {
  class Type;
}

namespace language {
  class AssociatedConformance;
  class AssociatedTypeDecl;
  class CanType;
  class FuncDecl;
  enum class MetadataState : size_t;
  class ProtocolConformanceRef;
  struct SILDeclRef;
  class SILType;
  class SILFunction;

namespace irgen {
  class Address;
  class DynamicMetadataRequest;
  class EntryPointArgumentEmission;
  class Explosion;
  class FunctionPointer;
  class IRGenFunction;
  class IRGenModule;
  class MetadataPath;
  class MetadataResponse;
  class NativeCCEntryPointArgumentEmission;
  class PolymorphicSignatureExpandedTypeSource;
  class ProtocolInfo;
  class TypeInfo;

  /// Set an LLVM value name for the given type metadata.
  void setTypeMetadataName(IRGenModule &IGM, toolchain::Value *value, CanType type);

  /// Set an LLVM value name for the given protocol witness table.
  void setProtocolWitnessTableName(IRGenModule &IGM, toolchain::Value *value,
                                   CanType type, ProtocolDecl *protocol);

  /// Extract the method pointer from the given witness table
  /// as a function value.
  FunctionPointer emitWitnessMethodValue(IRGenFunction &IGF,
                                         toolchain::Value *wtable,
                                         SILDeclRef member);

  /// Extract the method pointer from an archetype's witness table
  /// as a function value.
  FunctionPointer emitWitnessMethodValue(IRGenFunction &IGF, CanType baseTy,
                                         toolchain::Value **baseMetadataCache,
                                         SILDeclRef member,
                                         ProtocolConformanceRef conformance);

  toolchain::Value *emitAssociatedConformanceValue(IRGenFunction &IGF,
                                              toolchain::Value *wtable,
                                              const AssociatedConformance &conf);

  /// Compute the index into a witness table for a resilient protocol given
  /// a reference to a descriptor of one of the requirements in that witness
  /// table.
  toolchain::Value *computeResilientWitnessTableIndex(
                                            IRGenFunction &IGF,
                                            ProtocolDecl *proto,
                                            toolchain::Constant *reqtDescriptor);

  /// Given a type T and an associated type X of some protocol P to
  /// which T conforms, return the type metadata for T.X.
  ///
  /// \param parentMetadata - the type metadata for T
  /// \param wtable - the witness table witnessing the conformance of T to P
  /// \param assocType - the declaration of X; a member of P
  MetadataResponse emitAssociatedTypeMetadataRef(IRGenFunction &IGF,
                                                 toolchain::Value *parentMetadata,
                                                 toolchain::Value *wtable,
                                                 AssociatedTypeDecl *assocType,
                                                 DynamicMetadataRequest request);

  // Return the offset one should do on a witness table pointer to retrieve the
  // `index`th piece of private data.
  inline int privateWitnessTableIndexToTableOffset(unsigned index) {
    return -1 - (int)index;
  }

  toolchain::Value *loadParentProtocolWitnessTable(IRGenFunction &IGF,
                                              toolchain::Value *wtable,
                                              WitnessIndex index);

  toolchain::Value *loadConditionalConformance(IRGenFunction &IGF,
                                          toolchain::Value *wtable,
                                          WitnessIndex index);

  struct ExpandedSignature {
    unsigned numShapes;
    unsigned numTypeMetadataPtrs;
    unsigned numWitnessTablePtrs;
    unsigned numValues;
  };

  /// Add the witness parameters necessary for calling a function with
  /// the given generics clause.
  /// Returns the number of lowered parameters of each kind.
  ExpandedSignature expandPolymorphicSignature(
      IRGenModule &IGM, CanSILFunctionType type,
      SmallVectorImpl<toolchain::Type *> &types,
      SmallVectorImpl<PolymorphicSignatureExpandedTypeSource> *outReqs =
          nullptr);

  /// Return the number of trailing arguments necessary for calling a
  /// witness method.
  inline unsigned getTrailingWitnessSignatureLength(IRGenModule &IGM,
                                                    CanSILFunctionType type) {
    return 2;
  }

  /// Add the trailing arguments necessary for calling a witness method.
  void expandTrailingWitnessSignature(IRGenModule &IGM,
                                      CanSILFunctionType type,
                                      SmallVectorImpl<toolchain::Type*> &types);

  struct WitnessMetadata {
    toolchain::Value *SelfMetadata = nullptr;
    toolchain::Value *SelfWitnessTable = nullptr;
  };

  /// Collect any required metadata for a witness method from the end
  /// of the given parameter list.
  void
  collectTrailingWitnessMetadata(IRGenFunction &IGF, SILFunction &fn,
                                 NativeCCEntryPointArgumentEmission &params,
                                 WitnessMetadata &metadata);

  using GetParameterFn = toolchain::function_ref<toolchain::Value*(unsigned)>;

  /// In the prelude of a generic function, perform the bindings for a
  /// generics clause.
  ///
  /// \param witnessMetadata - can be omitted if the function is
  ///   definitely not a witness method
  void emitPolymorphicParameters(IRGenFunction &IGF, SILFunction &Fn,
                                 EntryPointArgumentEmission &emission,
                                 WitnessMetadata *witnessMetadata,
                                 const GetParameterFn &getParameter);

  void emitPolymorphicParametersFromArray(IRGenFunction &IGF,
                                          NominalTypeDecl *typeDecl,
                                          Address array,
                                          MetadataState metadataState);

  /// When calling a polymorphic call, pass the arguments for the
  /// generics clause.
  void emitPolymorphicArguments(IRGenFunction &IGF,
                                CanSILFunctionType origType,
                                SubstitutionMap subs,
                                WitnessMetadata *witnessMetadata,
                                Explosion &args);

  /// Bind the polymorphic parameter inside of a partial apply forwarding thunk.
  void bindPolymorphicParameter(IRGenFunction &IGF,
                                CanSILFunctionType &OrigFnType,
                                CanSILFunctionType &SubstFnType,
                                Explosion &nativeParam, unsigned paramIndex);

  /// Load a reference to the protocol descriptor for the given protocol.
  ///
  /// For Codira protocols, this is a constant reference to the protocol
  /// descriptor symbol.
  /// For ObjC protocols, descriptors are uniqued at runtime by the ObjC
  /// runtime. We need to load the unique reference from a global variable fixed up at
  /// startup.
  toolchain::Value *emitProtocolDescriptorRef(IRGenFunction &IGF,
                                         ProtocolDecl *protocol);

  /// Emit a witness table reference.
  toolchain::Value *emitWitnessTableRef(IRGenFunction &IGF,
                                   CanType srcType,
                                   toolchain::Value **srcMetadataCache,
                                   ProtocolConformanceRef conformance);

  toolchain::Value *emitWitnessTableRef(IRGenFunction &IGF,
                                   CanType srcType,
                                   ProtocolConformanceRef conformance);

  using GenericParamFulfillmentCallback =
    toolchain::function_ref<void(GenericRequirement req,
                            const MetadataSource &source,
                            const MetadataPath &path)>;

  void enumerateGenericParamFulfillments(IRGenModule &IGM,
    CanSILFunctionType fnType,
    GenericParamFulfillmentCallback callback);
} // end namespace irgen
} // end namespace language

#endif
