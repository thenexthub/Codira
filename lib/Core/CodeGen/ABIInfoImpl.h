//===- ABIInfoImpl.h --------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_CODEGEN_ABIINFOIMPL_H
#define LANGUAGE_CORE_LIB_CODEGEN_ABIINFOIMPL_H

#include "ABIInfo.h"
#include "CGCXXABI.h"

namespace language::Core::CodeGen {

/// DefaultABIInfo - The default implementation for ABI specific
/// details. This implementation provides information which results in
/// self-consistent and sensible LLVM IR generation, but does not
/// conform to any particular ABI.
class DefaultABIInfo : public ABIInfo {
public:
  DefaultABIInfo(CodeGen::CodeGenTypes &CGT) : ABIInfo(CGT) {}

  virtual ~DefaultABIInfo();

  ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType RetTy) const;

  void computeInfo(CGFunctionInfo &FI) const override;

  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override;
};

void AssignToArrayRange(CodeGen::CGBuilderTy &Builder, toolchain::Value *Array,
                        toolchain::Value *Value, unsigned FirstIndex,
                        unsigned LastIndex);

bool isAggregateTypeForABI(QualType T);

toolchain::Type *getVAListElementType(CodeGenFunction &CGF);

CGCXXABI::RecordArgABI getRecordArgABI(const RecordType *RT, CGCXXABI &CXXABI);

CGCXXABI::RecordArgABI getRecordArgABI(QualType T, CGCXXABI &CXXABI);

bool classifyReturnType(const CGCXXABI &CXXABI, CGFunctionInfo &FI,
                        const ABIInfo &Info);

/// Pass transparent unions as if they were the type of the first element. Sema
/// should ensure that all elements of the union have the same "machine type".
QualType useFirstFieldIfTransparentUnion(QualType Ty);

// Dynamically round a pointer up to a multiple of the given alignment.
toolchain::Value *emitRoundPointerUpToAlignment(CodeGenFunction &CGF,
                                           toolchain::Value *Ptr, CharUnits Align);

/// Emit va_arg for a platform using the common void* representation,
/// where arguments are simply emitted in an array of slots on the stack.
///
/// This version implements the core direct-value passing rules.
///
/// \param SlotSize - The size and alignment of a stack slot.
///   Each argument will be allocated to a multiple of this number of
///   slots, and all the slots will be aligned to this value.
/// \param AllowHigherAlign - The slot alignment is not a cap;
///   an argument type with an alignment greater than the slot size
///   will be emitted on a higher-alignment address, potentially
///   leaving one or more empty slots behind as padding.  If this
///   is false, the returned address might be less-aligned than
///   DirectAlign.
/// \param ForceRightAdjust - Default is false. On big-endian platform and
///   if the argument is smaller than a slot, set this flag will force
///   right-adjust the argument in its slot irrespective of the type.
Address emitVoidPtrDirectVAArg(CodeGenFunction &CGF, Address VAListAddr,
                               toolchain::Type *DirectTy, CharUnits DirectSize,
                               CharUnits DirectAlign, CharUnits SlotSize,
                               bool AllowHigherAlign,
                               bool ForceRightAdjust = false);

/// Emit va_arg for a platform using the common void* representation,
/// where arguments are simply emitted in an array of slots on the stack.
///
/// \param IsIndirect - Values of this type are passed indirectly.
/// \param ValueInfo - The size and alignment of this type, generally
///   computed with getContext().getTypeInfoInChars(ValueTy).
/// \param SlotSizeAndAlign - The size and alignment of a stack slot.
///   Each argument will be allocated to a multiple of this number of
///   slots, and all the slots will be aligned to this value.
/// \param AllowHigherAlign - The slot alignment is not a cap;
///   an argument type with an alignment greater than the slot size
///   will be emitted on a higher-alignment address, potentially
///   leaving one or more empty slots behind as padding.
/// \param ForceRightAdjust - Default is false. On big-endian platform and
///   if the argument is smaller than a slot, set this flag will force
///   right-adjust the argument in its slot irrespective of the type.
RValue emitVoidPtrVAArg(CodeGenFunction &CGF, Address VAListAddr,
                        QualType ValueTy, bool IsIndirect,
                        TypeInfoChars ValueInfo, CharUnits SlotSizeAndAlign,
                        bool AllowHigherAlign, AggValueSlot Slot,
                        bool ForceRightAdjust = false);

Address emitMergePHI(CodeGenFunction &CGF, Address Addr1,
                     toolchain::BasicBlock *Block1, Address Addr2,
                     toolchain::BasicBlock *Block2, const toolchain::Twine &Name = "");

/// isEmptyField - Return true iff a the field is "empty", that is it
/// is an unnamed bit-field or an (array of) empty record(s). If
/// AsIfNoUniqueAddr is true, then C++ record fields are considered empty if
/// the [[no_unique_address]] attribute would have made them empty.
bool isEmptyField(ASTContext &Context, const FieldDecl *FD, bool AllowArrays,
                  bool AsIfNoUniqueAddr = false);

/// isEmptyRecord - Return true iff a structure contains only empty
/// fields. Note that a structure with a flexible array member is not
/// considered empty. If AsIfNoUniqueAddr is true, then C++ record fields are
/// considered empty if the [[no_unique_address]] attribute would have made
/// them empty.
bool isEmptyRecord(ASTContext &Context, QualType T, bool AllowArrays,
                   bool AsIfNoUniqueAddr = false);

/// isEmptyFieldForLayout - Return true iff the field is "empty", that is,
/// either a zero-width bit-field or an \ref isEmptyRecordForLayout.
bool isEmptyFieldForLayout(const ASTContext &Context, const FieldDecl *FD);

/// isEmptyRecordForLayout - Return true iff a structure contains only empty
/// base classes (per \ref isEmptyRecordForLayout) and fields (per
/// \ref isEmptyFieldForLayout). Note, C++ record fields are considered empty
/// if the [[no_unique_address]] attribute would have made them empty.
bool isEmptyRecordForLayout(const ASTContext &Context, QualType T);

/// isSingleElementStruct - Determine if a structure is a "single
/// element struct", i.e. it has exactly one non-empty field or
/// exactly one field which is itself a single element
/// struct. Structures with flexible array members are never
/// considered single element structs.
///
/// \return The field declaration for the single non-empty field, if
/// it exists.
const Type *isSingleElementStruct(QualType T, ASTContext &Context);

Address EmitVAArgInstr(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                       const ABIArgInfo &AI);

bool isSIMDVectorType(ASTContext &Context, QualType Ty);

bool isRecordWithSIMDVectorType(ASTContext &Context, QualType Ty);

} // namespace language::Core::CodeGen

#endif // LANGUAGE_CORE_LIB_CODEGEN_ABIINFOIMPL_H
