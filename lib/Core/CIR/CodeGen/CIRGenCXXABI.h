//===----------------------------------------------------------------------===//
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
// This provides an abstract class for C++ code generation. Concrete subclasses
// of this implement code generation for specific C++ ABIs.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CIR_CIRGENCXXABI_H
#define LANGUAGE_CORE_LIB_CIR_CIRGENCXXABI_H

#include "CIRGenCall.h"
#include "CIRGenFunction.h"
#include "CIRGenModule.h"

#include "language/Core/AST/Mangle.h"

namespace language::Core::CIRGen {

/// Implements C++ ABI-specific code generation functions.
class CIRGenCXXABI {
protected:
  CIRGenModule &cgm;
  std::unique_ptr<language::Core::MangleContext> mangleContext;

public:
  // TODO(cir): make this protected when target-specific CIRGenCXXABIs are
  // implemented.
  CIRGenCXXABI(CIRGenModule &cgm)
      : cgm(cgm), mangleContext(cgm.getASTContext().createMangleContext()) {}
  virtual ~CIRGenCXXABI();

  void setCXXABIThisValue(CIRGenFunction &cgf, mlir::Value thisPtr);

  /// Emit a single constructor/destructor with the gen type from a C++
  /// constructor/destructor Decl.
  virtual void emitCXXStructor(language::Core::GlobalDecl gd) = 0;

public:
  language::Core::ImplicitParamDecl *getThisDecl(CIRGenFunction &cgf) {
    return cgf.cxxabiThisDecl;
  }

  /// Emit the ABI-specific prolog for the function
  virtual void emitInstanceFunctionProlog(SourceLocation Loc,
                                          CIRGenFunction &cgf) = 0;

  /// Get the type of the implicit "this" parameter used by a method. May return
  /// zero if no specific type is applicable, e.g. if the ABI expects the "this"
  /// parameter to point to some artificial offset in a complete object due to
  /// vbases being reordered.
  virtual const language::Core::CXXRecordDecl *
  getThisArgumentTypeForMethod(const language::Core::CXXMethodDecl *md) {
    return md->getParent();
  }

  /// Return whether the given global decl needs a VTT (virtual table table)
  /// parameter.
  virtual bool needsVTTParameter(language::Core::GlobalDecl gd) { return false; }

  /// Build a parameter variable suitable for 'this'.
  void buildThisParam(CIRGenFunction &cgf, FunctionArgList &params);

  /// Loads the incoming C++ this pointer as it was passed by the caller.
  mlir::Value loadIncomingCXXThis(CIRGenFunction &cgf);

  /// Emit constructor variants required by this ABI.
  virtual void emitCXXConstructors(const language::Core::CXXConstructorDecl *d) = 0;

  /// Emit dtor variants required by this ABI.
  virtual void emitCXXDestructors(const language::Core::CXXDestructorDecl *d) = 0;

  virtual void emitDestructorCall(CIRGenFunction &cgf,
                                  const CXXDestructorDecl *dd, CXXDtorType type,
                                  bool forVirtualBase, bool delegating,
                                  Address thisAddr, QualType thisTy) = 0;

  /// Checks if ABI requires extra virtual offset for vtable field.
  virtual bool
  isVirtualOffsetNeededForVTableField(CIRGenFunction &cgf,
                                      CIRGenFunction::VPtr vptr) = 0;

  /// Returns true if the given destructor type should be emitted as a linkonce
  /// delegating thunk, regardless of whether the dtor is defined in this TU or
  /// not.
  virtual bool useThunkForDtorVariant(const CXXDestructorDecl *dtor,
                                      CXXDtorType dt) const = 0;

  virtual cir::GlobalLinkageKind
  getCXXDestructorLinkage(GVALinkage linkage, const CXXDestructorDecl *dtor,
                          CXXDtorType dt) const;

  /// Get the address of the vtable for the given record decl which should be
  /// used for the vptr at the given offset in RD.
  virtual cir::GlobalOp getAddrOfVTable(const CXXRecordDecl *rd,
                                        CharUnits vptrOffset) = 0;

  /// Get the address point of the vtable for the given base subobject.
  virtual mlir::Value
  getVTableAddressPoint(BaseSubobject base,
                        const CXXRecordDecl *vtableClass) = 0;

  /// Get the address point of the vtable for the given base subobject while
  /// building a constructor or a destructor.
  virtual mlir::Value getVTableAddressPointInStructor(
      CIRGenFunction &cgf, const CXXRecordDecl *vtableClass, BaseSubobject base,
      const CXXRecordDecl *nearestVBase) = 0;

  /// Checks if ABI requires to initialize vptrs for given dynamic class.
  virtual bool
  doStructorsInitializeVPtrs(const language::Core::CXXRecordDecl *vtableClass) = 0;

  /// Returns true if the given constructor or destructor is one of the kinds
  /// that the ABI says returns 'this' (only applies when called non-virtually
  /// for destructors).
  ///
  /// There currently is no way to indicate if a destructor returns 'this' when
  /// called virtually, and CIR generation does not support this case.
  virtual bool hasThisReturn(language::Core::GlobalDecl gd) const { return false; }

  virtual bool hasMostDerivedReturn(language::Core::GlobalDecl gd) const {
    return false;
  }

  /// Gets the mangle context.
  language::Core::MangleContext &getMangleContext() { return *mangleContext; }
};

/// Creates and Itanium-family ABI
CIRGenCXXABI *CreateCIRGenItaniumCXXABI(CIRGenModule &cgm);

} // namespace language::Core::CIRGen

#endif
