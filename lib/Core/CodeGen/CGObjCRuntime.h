//===----- CGObjCRuntime.h - Interface to ObjC Runtimes ---------*- C++ -*-===//
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
// This provides an abstract class for Objective-C code generation.  Concrete
// subclasses of this implement code generation for specific Objective-C
// runtime libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGOBJCRUNTIME_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGOBJCRUNTIME_H
#include "CGBuilder.h"
#include "CGCall.h"
#include "CGCleanup.h"
#include "CGValue.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/Basic/IdentifierTable.h" // Selector
#include "toolchain/ADT/UniqueVector.h"

namespace toolchain {
class Constant;
class Function;
class Module;
class StructLayout;
class StructType;
class Type;
class Value;
} // namespace toolchain

namespace language::Core {
namespace CodeGen {
class CGFunctionInfo;
class CodeGenFunction;
} // namespace CodeGen

class FieldDecl;
class ObjCAtTryStmt;
class ObjCAtThrowStmt;
class ObjCAtSynchronizedStmt;
class ObjCContainerDecl;
class ObjCCategoryImplDecl;
class ObjCImplementationDecl;
class ObjCInterfaceDecl;
class ObjCMessageExpr;
class ObjCMethodDecl;
class ObjCProtocolDecl;
class Selector;
class ObjCIvarDecl;
class ObjCStringLiteral;
class BlockDeclRefExpr;

namespace CodeGen {
class CodeGenModule;
class CGBlockInfo;

// FIXME: Several methods should be pure virtual but aren't to avoid the
// partially-implemented subclass breaking.

/// Implements runtime-specific code generation functions.
class CGObjCRuntime {
protected:
  CodeGen::CodeGenModule &CGM;
  CGObjCRuntime(CodeGen::CodeGenModule &CGM) : CGM(CGM) {}

  // Utility functions for unified ivar access. These need to
  // eventually be folded into other places (the structure layout
  // code).

  /// Compute an offset to the given ivar, suitable for passing to
  /// EmitValueForIvarAtOffset.  Note that the correct handling of
  /// bit-fields is carefully coordinated by these two, use caution!
  ///
  /// The latter overload is suitable for computing the offset of a
  /// sythesized ivar.
  uint64_t ComputeIvarBaseOffset(CodeGen::CodeGenModule &CGM,
                                 const ObjCInterfaceDecl *OID,
                                 const ObjCIvarDecl *Ivar);
  uint64_t ComputeIvarBaseOffset(CodeGen::CodeGenModule &CGM,
                                 const ObjCImplementationDecl *OID,
                                 const ObjCIvarDecl *Ivar);

  LValue EmitValueForIvarAtOffset(CodeGen::CodeGenFunction &CGF,
                                  const ObjCInterfaceDecl *OID,
                                  toolchain::Value *BaseValue,
                                  const ObjCIvarDecl *Ivar,
                                  unsigned CVRQualifiers, toolchain::Value *Offset);
  /// Emits a try / catch statement.  This function is intended to be called by
  /// subclasses, and provides a generic mechanism for generating these, which
  /// should be usable by all runtimes.  The caller must provide the functions
  /// to call when entering and exiting a \@catch() block, and the function
  /// used to rethrow exceptions.  If the begin and end catch functions are
  /// NULL, then the function assumes that the EH personality function provides
  /// the thrown object directly.
  void EmitTryCatchStmt(CodeGenFunction &CGF, const ObjCAtTryStmt &S,
                        toolchain::FunctionCallee beginCatchFn,
                        toolchain::FunctionCallee endCatchFn,
                        toolchain::FunctionCallee exceptionRethrowFn);

  void EmitInitOfCatchParam(CodeGenFunction &CGF, toolchain::Value *exn,
                            const VarDecl *paramDecl);

  /// Emits an \@synchronize() statement, using the \p syncEnterFn and
  /// \p syncExitFn arguments as the functions called to lock and unlock
  /// the object.  This function can be called by subclasses that use
  /// zero-cost exception handling.
  void EmitAtSynchronizedStmt(CodeGenFunction &CGF,
                              const ObjCAtSynchronizedStmt &S,
                              toolchain::FunctionCallee syncEnterFn,
                              toolchain::FunctionCallee syncExitFn);

public:
  virtual ~CGObjCRuntime();

  std::string getSymbolNameForMethod(const ObjCMethodDecl *method,
                                     bool includeCategoryName = true);

  /// Generate the function required to register all Objective-C components in
  /// this compilation unit with the runtime library.
  virtual toolchain::Function *ModuleInitFunction() = 0;

  /// Get a selector for the specified name and type values.
  /// The result should have the LLVM type for ASTContext::getObjCSelType().
  virtual toolchain::Value *GetSelector(CodeGenFunction &CGF, Selector Sel) = 0;

  /// Get the address of a selector for the specified name and type values.
  /// This is a rarely-used language extension, but sadly it exists.
  ///
  /// The result should have the LLVM type for a pointer to
  /// ASTContext::getObjCSelType().
  virtual Address GetAddrOfSelector(CodeGenFunction &CGF, Selector Sel) = 0;

  /// Get a typed selector.
  virtual toolchain::Value *GetSelector(CodeGenFunction &CGF,
                                   const ObjCMethodDecl *Method) = 0;

  /// Get the type constant to catch for the given ObjC pointer type.
  /// This is used externally to implement catching ObjC types in C++.
  /// Runtimes which don't support this should add the appropriate
  /// error to Sema.
  virtual toolchain::Constant *GetEHType(QualType T) = 0;

  virtual CatchTypeInfo getCatchAllTypeInfo() { return {nullptr, 0}; }

  /// Generate a constant string object.
  virtual ConstantAddress GenerateConstantString(const StringLiteral *) = 0;

  /// Generate a category.  A category contains a list of methods (and
  /// accompanying metadata) and a list of protocols.
  virtual void GenerateCategory(const ObjCCategoryImplDecl *OCD) = 0;

  /// Generate a class structure for this class.
  virtual void GenerateClass(const ObjCImplementationDecl *OID) = 0;

  /// Register an class alias.
  virtual void RegisterAlias(const ObjCCompatibleAliasDecl *OAD) = 0;

  /// Generate an Objective-C message send operation.
  ///
  /// \param Method - The method being called, this may be null if synthesizing
  /// a property setter or getter.
  virtual CodeGen::RValue
  GenerateMessageSend(CodeGen::CodeGenFunction &CGF, ReturnValueSlot ReturnSlot,
                      QualType ResultType, Selector Sel, toolchain::Value *Receiver,
                      const CallArgList &CallArgs,
                      const ObjCInterfaceDecl *Class = nullptr,
                      const ObjCMethodDecl *Method = nullptr) = 0;

  /// Generate an Objective-C message send operation.
  ///
  /// This variant allows for the call to be substituted with an optimized
  /// variant.
  CodeGen::RValue GeneratePossiblySpecializedMessageSend(
      CodeGenFunction &CGF, ReturnValueSlot Return, QualType ResultType,
      Selector Sel, toolchain::Value *Receiver, const CallArgList &Args,
      const ObjCInterfaceDecl *OID, const ObjCMethodDecl *Method,
      bool isClassMessage);

  /// Generate an Objective-C message send operation to the super
  /// class initiated in a method for Class and with the given Self
  /// object.
  ///
  /// \param Method - The method being called, this may be null if synthesizing
  /// a property setter or getter.
  virtual CodeGen::RValue GenerateMessageSendSuper(
      CodeGen::CodeGenFunction &CGF, ReturnValueSlot ReturnSlot,
      QualType ResultType, Selector Sel, const ObjCInterfaceDecl *Class,
      bool isCategoryImpl, toolchain::Value *Self, bool IsClassMessage,
      const CallArgList &CallArgs, const ObjCMethodDecl *Method = nullptr) = 0;

  /// Walk the list of protocol references from a class, category or
  /// protocol to traverse the DAG formed from it's inheritance hierarchy. Find
  /// the list of protocols that ends each walk at either a runtime
  /// protocol or a non-runtime protocol with no parents. For the common case of
  /// just a list of standard runtime protocols this just returns the same list
  /// that was passed in.
  std::vector<const ObjCProtocolDecl *>
  GetRuntimeProtocolList(ObjCProtocolDecl::protocol_iterator begin,
                         ObjCProtocolDecl::protocol_iterator end);

  /// Emit the code to return the named protocol as an object, as in a
  /// \@protocol expression.
  virtual toolchain::Value *GenerateProtocolRef(CodeGenFunction &CGF,
                                           const ObjCProtocolDecl *OPD) = 0;

  /// Generate the named protocol.  Protocols contain method metadata but no
  /// implementations.
  virtual void GenerateProtocol(const ObjCProtocolDecl *OPD) = 0;

  /// GetOrEmitProtocol - Get the protocol object for the given
  /// declaration, emitting it if necessary. The return value has type
  /// ProtocolPtrTy.
  virtual toolchain::Constant *GetOrEmitProtocol(const ObjCProtocolDecl *PD) = 0;

  /// Generate a function preamble for a method with the specified
  /// types.

  // FIXME: Current this just generates the Function definition, but really this
  // should also be generating the loads of the parameters, as the runtime
  // should have full control over how parameters are passed.
  virtual toolchain::Function *GenerateMethod(const ObjCMethodDecl *OMD,
                                         const ObjCContainerDecl *CD) = 0;

  /// Generates prologue for direct Objective-C Methods.
  virtual void GenerateDirectMethodPrologue(CodeGenFunction &CGF,
                                            toolchain::Function *Fn,
                                            const ObjCMethodDecl *OMD,
                                            const ObjCContainerDecl *CD) = 0;

  /// Return the runtime function for getting properties.
  virtual toolchain::FunctionCallee GetPropertyGetFunction() = 0;

  /// Return the runtime function for setting properties.
  virtual toolchain::FunctionCallee GetPropertySetFunction() = 0;

  /// Return the runtime function for optimized setting properties.
  virtual toolchain::FunctionCallee GetOptimizedPropertySetFunction(bool atomic,
                                                               bool copy) = 0;

  // API for atomic copying of qualified aggregates in getter.
  virtual toolchain::FunctionCallee GetGetStructFunction() = 0;
  // API for atomic copying of qualified aggregates in setter.
  virtual toolchain::FunctionCallee GetSetStructFunction() = 0;
  /// API for atomic copying of qualified aggregates with non-trivial copy
  /// assignment (c++) in setter.
  virtual toolchain::FunctionCallee GetCppAtomicObjectSetFunction() = 0;
  /// API for atomic copying of qualified aggregates with non-trivial copy
  /// assignment (c++) in getter.
  virtual toolchain::FunctionCallee GetCppAtomicObjectGetFunction() = 0;

  /// GetClass - Return a reference to the class for the given
  /// interface decl.
  virtual toolchain::Value *GetClass(CodeGenFunction &CGF,
                                const ObjCInterfaceDecl *OID) = 0;

  virtual toolchain::Value *EmitNSAutoreleasePoolClassRef(CodeGenFunction &CGF) {
    toolchain_unreachable("autoreleasepool unsupported in this ABI");
  }

  /// EnumerationMutationFunction - Return the function that's called by the
  /// compiler when a mutation is detected during foreach iteration.
  virtual toolchain::FunctionCallee EnumerationMutationFunction() = 0;

  virtual void EmitSynchronizedStmt(CodeGen::CodeGenFunction &CGF,
                                    const ObjCAtSynchronizedStmt &S) = 0;
  virtual void EmitTryStmt(CodeGen::CodeGenFunction &CGF,
                           const ObjCAtTryStmt &S) = 0;
  virtual void EmitThrowStmt(CodeGen::CodeGenFunction &CGF,
                             const ObjCAtThrowStmt &S,
                             bool ClearInsertionPoint = true) = 0;
  virtual toolchain::Value *EmitObjCWeakRead(CodeGen::CodeGenFunction &CGF,
                                        Address AddrWeakObj) = 0;
  virtual void EmitObjCWeakAssign(CodeGen::CodeGenFunction &CGF,
                                  toolchain::Value *src, Address dest) = 0;
  virtual void EmitObjCGlobalAssign(CodeGen::CodeGenFunction &CGF,
                                    toolchain::Value *src, Address dest,
                                    bool threadlocal = false) = 0;
  virtual void EmitObjCIvarAssign(CodeGen::CodeGenFunction &CGF,
                                  toolchain::Value *src, Address dest,
                                  toolchain::Value *ivarOffset) = 0;
  virtual void EmitObjCStrongCastAssign(CodeGen::CodeGenFunction &CGF,
                                        toolchain::Value *src, Address dest) = 0;

  virtual LValue EmitObjCValueForIvar(CodeGen::CodeGenFunction &CGF,
                                      QualType ObjectTy, toolchain::Value *BaseValue,
                                      const ObjCIvarDecl *Ivar,
                                      unsigned CVRQualifiers) = 0;
  virtual toolchain::Value *EmitIvarOffset(CodeGen::CodeGenFunction &CGF,
                                      const ObjCInterfaceDecl *Interface,
                                      const ObjCIvarDecl *Ivar) = 0;
  virtual void EmitGCMemmoveCollectable(CodeGen::CodeGenFunction &CGF,
                                        Address DestPtr, Address SrcPtr,
                                        toolchain::Value *Size) = 0;
  virtual toolchain::Constant *
  BuildGCBlockLayout(CodeGen::CodeGenModule &CGM,
                     const CodeGen::CGBlockInfo &blockInfo) = 0;
  virtual toolchain::Constant *
  BuildRCBlockLayout(CodeGen::CodeGenModule &CGM,
                     const CodeGen::CGBlockInfo &blockInfo) = 0;
  virtual std::string getRCBlockLayoutStr(CodeGen::CodeGenModule &CGM,
                                          const CGBlockInfo &blockInfo) {
    return {};
  }

  /// Returns an i8* which points to the byref layout information.
  virtual toolchain::Constant *BuildByrefLayout(CodeGen::CodeGenModule &CGM,
                                           QualType T) = 0;

  struct MessageSendInfo {
    const CGFunctionInfo &CallInfo;
    toolchain::PointerType *MessengerType;

    MessageSendInfo(const CGFunctionInfo &callInfo,
                    toolchain::PointerType *messengerType)
        : CallInfo(callInfo), MessengerType(messengerType) {}
  };

  MessageSendInfo getMessageSendInfo(const ObjCMethodDecl *method,
                                     QualType resultType,
                                     CallArgList &callArgs);
  bool canMessageReceiverBeNull(CodeGenFunction &CGF,
                                const ObjCMethodDecl *method, bool isSuper,
                                const ObjCInterfaceDecl *classReceiver,
                                toolchain::Value *receiver);
  static bool isWeakLinkedClass(const ObjCInterfaceDecl *cls);

  /// Destroy the callee-destroyed arguments of the given method,
  /// if it has any.  Used for nil-receiver paths in message sends.
  /// Never does anything if the method does not satisfy
  /// hasParamDestroyedInCallee().
  ///
  /// \param callArgs - just the formal arguments, not including implicit
  ///   arguments such as self and cmd
  static void destroyCalleeDestroyedArguments(CodeGenFunction &CGF,
                                              const ObjCMethodDecl *method,
                                              const CallArgList &callArgs);

  // FIXME: This probably shouldn't be here, but the code to compute
  // it is here.
  unsigned ComputeBitfieldBitOffset(CodeGen::CodeGenModule &CGM,
                                    const ObjCInterfaceDecl *ID,
                                    const ObjCIvarDecl *Ivar);
};

/// Creates an instance of an Objective-C runtime class.
// TODO: This should include some way of selecting which runtime to target.
CGObjCRuntime *CreateGNUObjCRuntime(CodeGenModule &CGM);
CGObjCRuntime *CreateMacObjCRuntime(CodeGenModule &CGM);
} // namespace CodeGen
} // namespace language::Core
#endif
