//===----- ABIInfo.h - ABI information access & encapsulation ---*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_CODEGEN_ABIINFO_H
#define LANGUAGE_CORE_LIB_CODEGEN_ABIINFO_H

#include "language/Core/AST/Attr.h"
#include "language/Core/AST/CharUnits.h"
#include "language/Core/AST/Type.h"
#include "toolchain/IR/CallingConv.h"
#include "toolchain/IR/Type.h"

namespace toolchain {
class Value;
class LLVMContext;
class DataLayout;
class Type;
class FixedVectorType;
} // namespace toolchain

namespace language::Core {
class ASTContext;
class CodeGenOptions;
class TargetInfo;

namespace CodeGen {
class ABIArgInfo;
class Address;
class CGCXXABI;
class CGFunctionInfo;
class CodeGenFunction;
class CodeGenTypes;
class RValue;
class AggValueSlot;

// FIXME: All of this stuff should be part of the target interface
// somehow. It is currently here because it is not clear how to factor
// the targets to support this, since the Targets currently live in a
// layer below types n'stuff.

/// ABIInfo - Target specific hooks for defining how a type should be
/// passed or returned from functions.
class ABIInfo {
protected:
  CodeGen::CodeGenTypes &CGT;
  toolchain::CallingConv::ID RuntimeCC;

public:
  ABIInfo(CodeGen::CodeGenTypes &cgt)
      : CGT(cgt), RuntimeCC(toolchain::CallingConv::C) {}

  virtual ~ABIInfo();

  virtual bool allowBFloatArgsAndRet() const { return false; }

  CodeGen::CGCXXABI &getCXXABI() const;
  ASTContext &getContext() const;
  toolchain::LLVMContext &getVMContext() const;
  const toolchain::DataLayout &getDataLayout() const;
  const TargetInfo &getTarget() const;
  const CodeGenOptions &getCodeGenOpts() const;

  /// Return the calling convention to use for system runtime
  /// functions.
  toolchain::CallingConv::ID getRuntimeCC() const { return RuntimeCC; }

  virtual void computeInfo(CodeGen::CGFunctionInfo &FI) const = 0;

  /// EmitVAArg - Emit the target dependent code to load a value of
  /// \arg Ty from the va_list pointed to by \arg VAListAddr.

  // FIXME: This is a gaping layering violation if we wanted to drop
  // the ABI information any lower than CodeGen. Of course, for
  // VAArg handling it has to be at this level; there is no way to
  // abstract this out.
  virtual RValue EmitVAArg(CodeGen::CodeGenFunction &CGF,
                           CodeGen::Address VAListAddr, QualType Ty,
                           AggValueSlot Slot) const = 0;

  bool isAndroid() const;
  bool isOHOSFamily() const;

  /// Emit the target dependent code to load a value of
  /// \arg Ty from the \c __builtin_ms_va_list pointed to by \arg VAListAddr.
  virtual RValue EmitMSVAArg(CodeGen::CodeGenFunction &CGF,
                             CodeGen::Address VAListAddr, QualType Ty,
                             AggValueSlot Slot) const;

  virtual bool isHomogeneousAggregateBaseType(QualType Ty) const;

  virtual bool isHomogeneousAggregateSmallEnough(const Type *Base,
                                                 uint64_t Members) const;
  virtual bool isZeroLengthBitfieldPermittedInHomogeneousAggregate() const;

  /// isHomogeneousAggregate - Return true if a type is an ELFv2 homogeneous
  /// aggregate.  Base is set to the base element type, and Members is set
  /// to the number of base elements.
  bool isHomogeneousAggregate(QualType Ty, const Type *&Base,
                              uint64_t &Members) const;

  // Implement the Type::IsPromotableIntegerType for ABI specific needs. The
  // only difference is that this considers bit-precise integer types as well.
  bool isPromotableIntegerTypeForABI(QualType Ty) const;

  /// A convenience method to return an indirect ABIArgInfo with an
  /// expected alignment equal to the ABI alignment of the given type.
  CodeGen::ABIArgInfo
  getNaturalAlignIndirect(QualType Ty, unsigned AddrSpace, bool ByVal = true,
                          bool Realign = false,
                          toolchain::Type *Padding = nullptr) const;

  CodeGen::ABIArgInfo getNaturalAlignIndirectInReg(QualType Ty,
                                                   bool Realign = false) const;

  virtual void appendAttributeMangling(TargetAttr *Attr,
                                       raw_ostream &Out) const;
  virtual void appendAttributeMangling(TargetVersionAttr *Attr,
                                       raw_ostream &Out) const;
  virtual void appendAttributeMangling(TargetClonesAttr *Attr, unsigned Index,
                                       raw_ostream &Out) const;
  virtual void appendAttributeMangling(StringRef AttrStr,
                                       raw_ostream &Out) const;

  /// Returns the optimal vector memory type based on the given vector type. For
  /// example, on certain targets, a vector with 3 elements might be promoted to
  /// one with 4 elements to improve performance.
  virtual toolchain::FixedVectorType *
  getOptimalVectorMemoryType(toolchain::FixedVectorType *T,
                             const LangOptions &Opt) const;
};

/// Target specific hooks for defining how a type should be passed or returned
/// from functions with one of the Swift calling conventions.
class SwiftABIInfo {
protected:
  CodeGenTypes &CGT;
  bool SwiftErrorInRegister;

  bool occupiesMoreThan(ArrayRef<toolchain::Type *> scalarTypes,
                        unsigned maxAllRegisters) const;

public:
  SwiftABIInfo(CodeGen::CodeGenTypes &CGT, bool SwiftErrorInRegister)
      : CGT(CGT), SwiftErrorInRegister(SwiftErrorInRegister) {}

  virtual ~SwiftABIInfo();

  /// Returns true if an aggregate which expands to the given type sequence
  /// should be passed / returned indirectly.
  virtual bool shouldPassIndirectly(ArrayRef<toolchain::Type *> ComponentTys,
                                    bool AsReturnValue) const;

  /// Returns true if the given vector type is legal from Swift's calling
  /// convention perspective.
  virtual bool isLegalVectorType(CharUnits VectorSize, toolchain::Type *EltTy,
                                 unsigned NumElts) const;

  /// Returns true if swifterror is lowered to a register by the target ABI.
  bool isSwiftErrorInRegister() const { return SwiftErrorInRegister; };
};
} // end namespace CodeGen
} // end namespace language::Core

#endif
