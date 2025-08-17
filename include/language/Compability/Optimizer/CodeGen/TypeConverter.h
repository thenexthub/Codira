//===-- TypeConverter.h -- type conversion ----------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_CODEGEN_TYPECONVERTER_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_CODEGEN_TYPECONVERTER_H

#include "language/Compability/Optimizer/Builder/Todo.h" // remove when TODO's are done
#include "language/Compability/Optimizer/CodeGen/TBAABuilder.h"
#include "language/Compability/Optimizer/CodeGen/Target.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Dialect/Support/FIRContext.h"
#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "toolchain/Support/Debug.h"

// Position of the different values in a `fir.box`.
static constexpr unsigned kAddrPosInBox = 0;
static constexpr unsigned kElemLenPosInBox = 1;
static constexpr unsigned kVersionPosInBox = 2;
static constexpr unsigned kRankPosInBox = 3;
static constexpr unsigned kTypePosInBox = 4;
static constexpr unsigned kAttributePosInBox = 5;
static constexpr unsigned kExtraPosInBox = 6;
static constexpr unsigned kDimsPosInBox = 7;
static constexpr unsigned kOptTypePtrPosInBox = 8;
static constexpr unsigned kOptRowTypePosInBox = 9;

// Position of the different values in [dims]
static constexpr unsigned kDimLowerBoundPos = 0;
static constexpr unsigned kDimExtentPos = 1;
static constexpr unsigned kDimStridePos = 2;

namespace mlir {
class DataLayout;
}

namespace fir {

/// FIR type converter
/// This converts FIR types to LLVM types (for now)
class LLVMTypeConverter : public mlir::LLVMTypeConverter {
public:
  LLVMTypeConverter(mlir::ModuleOp module, bool applyTBAA,
                    bool forceUnifiedTBAATree, const mlir::DataLayout &);

  // i32 is used here because LLVM wants i32 constants when indexing into struct
  // types. Indexing into other aggregate types is more flexible.
  mlir::Type offsetType() const;

  // i64 can be used to index into aggregates like arrays
  mlir::Type indexType() const;

  // fir.type<name(p : TY'...){f : TY...}>  -->  toolchain<"%name = { ty... }">
  std::optional<toolchain::LogicalResult>
  convertRecordType(fir::RecordType derived,
                    toolchain::SmallVectorImpl<mlir::Type> &results, bool isPacked);

  // Is an extended descriptor needed given the element type of a fir.box type ?
  // Extended descriptors are required for derived types.
  bool requiresExtendedDesc(mlir::Type boxElementType) const;

  // Magic value to indicate we do not know the rank of an entity, either
  // because it is assumed rank or because we have not determined it yet.
  static constexpr int unknownRank() { return -1; }

  // This corresponds to the descriptor as defined in ISO_Fortran_binding.h and
  // the addendum defined in descriptor.h.
  mlir::Type convertBoxType(BaseBoxType box, int rank = unknownRank()) const;

  /// Convert fir.box type to the corresponding toolchain struct type instead of a
  /// pointer to this struct type.
  mlir::Type convertBoxTypeAsStruct(BaseBoxType box, int = unknownRank()) const;

  // fir.boxproc<any>  -->  toolchain<"{ any*, i8* }">
  mlir::Type convertBoxProcType(BoxProcType boxproc) const;

  unsigned characterBitsize(fir::CharacterType charTy) const;

  // fir.char<k,?>  -->  toolchain<"ix">          where ix is scaled by kind mapping
  // fir.char<k,n>  -->  toolchain.array<n x "ix">
  mlir::Type convertCharType(fir::CharacterType charTy) const;

  template <typename A> mlir::Type convertPointerLike(A &ty) const {
    return mlir::LLVM::LLVMPointerType::get(ty.getContext());
  }

  // fir.array<c ... :any>  -->  toolchain<"[...[c x any]]">
  mlir::Type convertSequenceType(SequenceType seq) const;

  // fir.tdesc<any>  -->  toolchain<"i8*">
  // TODO: For now use a void*, however pointer identity is not sufficient for
  // the f18 object v. class distinction (F2003).
  mlir::Type convertTypeDescType(mlir::MLIRContext *ctx) const;

  const KindMapping &getKindMap() const { return kindMapping; }

  // Relay TBAA tag attachment to TBAABuilder.
  void attachTBAATag(mlir::LLVM::AliasAnalysisOpInterface op,
                     mlir::Type baseFIRType, mlir::Type accessFIRType,
                     mlir::LLVM::GEPOp gep) const;

  const mlir::DataLayout &getDataLayout() const {
    assert(dataLayout && "must be set in ctor");
    return *dataLayout;
  }

private:
  KindMapping kindMapping;
  std::unique_ptr<CodeGenSpecifics> specifics;
  std::unique_ptr<TBAABuilder> tbaaBuilder;
  const mlir::DataLayout *dataLayout;
};

} // namespace fir

#endif // FORTRAN_OPTIMIZER_CODEGEN_TYPECONVERTER_H
