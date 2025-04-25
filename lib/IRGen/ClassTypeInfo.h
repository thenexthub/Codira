//===--- ClassTypeInfo.h - The layout info for class types. -----*- C++ -*-===//
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
//  This file contains layout information for class types.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_CLASSTYPEINFO_H
#define SWIFT_IRGEN_CLASSTYPEINFO_H

#include "ClassLayout.h"
#include "HeapTypeInfo.h"

#include "language/ClangImporter/ClangImporterRequests.h"

namespace language {
namespace irgen {

/// Layout information for class types.
class ClassTypeInfo : public HeapTypeInfo<ClassTypeInfo> {
  ClassDecl *TheClass;
  mutable llvm::StructType *classLayoutType;

  // The resilient layout of the class, without making any assumptions
  // that violate resilience boundaries. This is used to allocate
  // and deallocate instances of the class, and to access fields.
  mutable std::optional<ClassLayout> ResilientLayout;

  // A completely fragile layout, used for metadata emission.
  mutable std::optional<ClassLayout> FragileLayout;

  /// Can we use swift reference-counting, or do we have to use
  /// objc_retain/release?
  const ReferenceCounting Refcount;

  ClassLayout generateLayout(IRGenModule &IGM, SILType classType,
                             bool forBackwardDeployment) const;

public:
  ClassTypeInfo(llvm::PointerType *irType, Size size, SpareBitVector spareBits,
                Alignment align, ClassDecl *theClass,
                ReferenceCounting refcount, llvm::StructType *classLayoutType)
      : HeapTypeInfo(refcount, irType, size, std::move(spareBits), align),
        TheClass(theClass), classLayoutType(classLayoutType),
        Refcount(refcount) {}

  ReferenceCounting getReferenceCounting() const { return Refcount; }

  ClassDecl *getClass() const { return TheClass; }

  llvm::Type *getClassLayoutType() const { return classLayoutType; }

  const ClassLayout &getClassLayout(IRGenModule &IGM, SILType type,
                                    bool forBackwardDeployment) const;

  StructLayout *createLayoutWithTailElems(IRGenModule &IGM, SILType classType,
                                          ArrayRef<SILType> tailTypes) const;

  void emitScalarRelease(IRGenFunction &IGF, llvm::Value *value,
                         Atomicity atomicity) const override {
    if (getReferenceCounting() == ReferenceCounting::Custom) {
      auto releaseFn =
          evaluateOrDefault(
              getClass()->getASTContext().evaluator,
              CustomRefCountingOperation(
                  {getClass(), CustomRefCountingOperationKind::release}),
              {})
              .operation;
      IGF.emitForeignReferenceTypeLifetimeOperation(releaseFn, value);
      return;
    }

    HeapTypeInfo::emitScalarRelease(IGF, value, atomicity);
  }

  void emitScalarRetain(IRGenFunction &IGF, llvm::Value *value,
                        Atomicity atomicity) const override {
    if (getReferenceCounting() == ReferenceCounting::Custom) {
      auto retainFn =
          evaluateOrDefault(
              getClass()->getASTContext().evaluator,
              CustomRefCountingOperation(
                  {getClass(), CustomRefCountingOperationKind::retain}),
              {})
              .operation;
      IGF.emitForeignReferenceTypeLifetimeOperation(retainFn, value);
      return;
    }

    HeapTypeInfo::emitScalarRetain(IGF, value, atomicity);
  }

  void strongCustomRetain(IRGenFunction &IGF, Explosion &e,
                          bool needsNullCheck) const {
    assert(getReferenceCounting() == ReferenceCounting::Custom &&
           "only supported for custom ref-counting");

    llvm::Value *value = e.claimNext();
    auto retainFn =
        evaluateOrDefault(
            getClass()->getASTContext().evaluator,
            CustomRefCountingOperation(
                {getClass(), CustomRefCountingOperationKind::retain}),
            {})
            .operation;
    IGF.emitForeignReferenceTypeLifetimeOperation(retainFn, value,
                                                  needsNullCheck);
  }

  // Implement the primary retain/release operations of ReferenceTypeInfo
  // using basic reference counting.
  void strongRetain(IRGenFunction &IGF, Explosion &e,
                    Atomicity atomicity) const override {
    if (getReferenceCounting() == ReferenceCounting::Custom) {
      strongCustomRetain(IGF, e, /*needsNullCheck*/ false);
      return;
    }

    HeapTypeInfo::strongRetain(IGF, e, atomicity);
  }

  void strongCustomRelease(IRGenFunction &IGF, Explosion &e,
                           bool needsNullCheck) const {
    assert(getReferenceCounting() == ReferenceCounting::Custom &&
           "only supported for custom ref-counting");

    llvm::Value *value = e.claimNext();
    auto releaseFn =
        evaluateOrDefault(
            getClass()->getASTContext().evaluator,
            CustomRefCountingOperation(
                {getClass(), CustomRefCountingOperationKind::release}),
            {})
            .operation;
    IGF.emitForeignReferenceTypeLifetimeOperation(releaseFn, value,
                                                  needsNullCheck);
  }

  void strongRelease(IRGenFunction &IGF, Explosion &e,
                     Atomicity atomicity) const override {
    if (getReferenceCounting() == ReferenceCounting::Custom) {
      strongCustomRelease(IGF, e, /*needsNullCheck*/ false);
      return;
    }

    HeapTypeInfo::strongRelease(IGF, e, atomicity);
  }
};

} // namespace irgen
} // namespace language

#endif
