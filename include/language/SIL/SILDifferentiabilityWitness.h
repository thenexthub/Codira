//===--- SILDifferentiabilityWitness.h - Differentiability witnesses ------===//
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
// This file defines the SILDifferentiabilityWitness class, which maps an
// original SILFunction and derivative configuration (parameter indices, result
// indices, derivative generic signature) to derivative functions (JVP and VJP).
//
// SIL differentiability witnesses are generated from the `@differentiable`
// and `@derivative` AST declaration attributes.
//
// Differentiability witnesses are canonicalized by the SIL differentiation
// transform, which fills in missing derivative functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILDIFFERENTIABILITYWITNESS_H
#define LANGUAGE_SIL_SILDIFFERENTIABILITYWITNESS_H

#include "language/AST/Attr.h"
#include "language/AST/AutoDiff.h"
#include "language/AST/GenericSignature.h"
#include "language/SIL/SILAllocated.h"
#include "language/SIL/SILLinkage.h"
#include "toolchain/ADT/ilist.h"
#include "toolchain/ADT/ilist_node.h"

namespace language {

class SILPrintContext;

class SILDifferentiabilityWitness
    : public toolchain::ilist_node<SILDifferentiabilityWitness>,
      public SILAllocated<SILDifferentiabilityWitness> {
private:
  /// The module which contains the differentiability witness.
  SILModule &Module;
  /// The linkage of the differentiability witness.
  SILLinkage Linkage;
  /// The original function.
  SILFunction *OriginalFunction;
  /// The differentiability kind.
  DifferentiabilityKind Kind;
  /// The derivative configuration: parameter indices, result indices, and
  /// derivative generic signature (optional). The derivative generic signature
  /// may contain same-type requirements such that all generic parameters are
  /// bound to concrete types.
  AutoDiffConfig Config;
  /// The JVP (Jacobian-vector products) derivative function.
  SILFunction *JVP;
  /// The VJP (vector-Jacobian products) derivative function.
  SILFunction *VJP;
  /// Whether or not this differentiability witness is a declaration.
  bool IsDeclaration;
  /// Whether or not this differentiability witness is serialized, which allows
  /// devirtualization from another module.
  bool IsSerialized;
  /// The AST `@differentiable` or `@derivative` attribute from which the
  /// differentiability witness is generated. Used for diagnostics.
  /// Null if the differentiability witness is parsed from SIL or if it is
  /// deserialized.
  const DeclAttribute *Attribute = nullptr;

  SILDifferentiabilityWitness(
      SILModule &module, SILLinkage linkage, SILFunction *originalFunction,
      DifferentiabilityKind kind, IndexSubset *parameterIndices,
      IndexSubset *resultIndices, GenericSignature derivativeGenSig,
      SILFunction *jvp, SILFunction *vjp, bool isDeclaration, bool isSerialized,
      const DeclAttribute *attribute)
      : Module(module), Linkage(linkage), OriginalFunction(originalFunction),
        Kind(kind),
        Config(parameterIndices, resultIndices, derivativeGenSig.getPointer()),
        JVP(jvp), VJP(vjp), IsDeclaration(isDeclaration),
        IsSerialized(isSerialized), Attribute(attribute) {
    assert(kind != DifferentiabilityKind::NonDifferentiable);
  }

public:
  static SILDifferentiabilityWitness *
  createDeclaration(SILModule &module, SILLinkage linkage,
                    SILFunction *originalFunction, DifferentiabilityKind kind,
                    IndexSubset *parameterIndices, IndexSubset *resultIndices,
                    GenericSignature derivativeGenSig,
                    const DeclAttribute *attribute = nullptr);

  static SILDifferentiabilityWitness *createDefinition(
      SILModule &module, SILLinkage linkage, SILFunction *originalFunction,
      DifferentiabilityKind kind, IndexSubset *parameterIndices,
      IndexSubset *resultIndices, GenericSignature derivativeGenSig,
      SILFunction *jvp, SILFunction *vjp, bool isSerialized,
      const DeclAttribute *attribute = nullptr);

  void convertToDefinition(SILFunction *jvp, SILFunction *vjp,
                           bool isSerialized);

  SILDifferentiabilityWitnessKey getKey() const;
  SILModule &getModule() const { return Module; }
  SILLinkage getLinkage() const { return Linkage; }
  SILFunction *getOriginalFunction() const { return OriginalFunction; }
  DifferentiabilityKind getKind() const { return Kind; }
  const AutoDiffConfig &getConfig() const { return Config; }
  IndexSubset *getParameterIndices() const { return Config.parameterIndices; }
  IndexSubset *getResultIndices() const { return Config.resultIndices; }
  GenericSignature getDerivativeGenericSignature() const {
    return Config.derivativeGenericSignature;
  }
  SILFunction *getJVP() const { return JVP; }
  SILFunction *getVJP() const { return VJP; }
  SILFunction *getDerivative(AutoDiffDerivativeFunctionKind kind) const {
    switch (kind) {
    case AutoDiffDerivativeFunctionKind::JVP:
      return JVP;
    case AutoDiffDerivativeFunctionKind::VJP:
      return VJP;
    }
    toolchain_unreachable("invalid derivative type");
  }
  void setJVP(SILFunction *jvp) { JVP = jvp; }
  void setVJP(SILFunction *vjp) { VJP = vjp; }
  void setDerivative(AutoDiffDerivativeFunctionKind kind,
                     SILFunction *derivative) {
    switch (kind) {
    case AutoDiffDerivativeFunctionKind::JVP:
      JVP = derivative;
      break;
    case AutoDiffDerivativeFunctionKind::VJP:
      VJP = derivative;
      break;
    }
  }
  bool isDeclaration() const { return IsDeclaration; }
  bool isDefinition() const { return !IsDeclaration; }
  bool isSerialized() const { return IsSerialized; }
  const DeclAttribute *getAttribute() const { return Attribute; }

  /// Verify that the differentiability witness is well-formed.
  void verify(const SILModule &module) const;

  void print(toolchain::raw_ostream &os, bool verbose = false) const;
  void dump() const;
};

} // end namespace language

namespace toolchain {

//===----------------------------------------------------------------------===//
// ilist_traits for SILDifferentiabilityWitness
//===----------------------------------------------------------------------===//

template <>
struct ilist_traits<::language::SILDifferentiabilityWitness>
    : public ilist_node_traits<::language::SILDifferentiabilityWitness> {
  using SILDifferentiabilityWitness = ::language::SILDifferentiabilityWitness;

public:
  static void deleteNode(SILDifferentiabilityWitness *DW) {
    DW->~SILDifferentiabilityWitness();
  }

private:
  void createNode(const SILDifferentiabilityWitness &);
};

} // namespace toolchain

#endif // LANGUAGE_SIL_SILDIFFERENTIABILITYWITNESS_H
