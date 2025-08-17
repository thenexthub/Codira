//===-- Lower/OpenMP/Utils.h ------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_OPENMPUTILS_H
#define LANGUAGE_COMPABILITY_LOWER_OPENMPUTILS_H

#include "language/Compability/Lower/OpenMP/Clauses.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/Value.h"
#include "toolchain/Support/CommandLine.h"
#include <cstdint>

extern toolchain::cl::opt<bool> treatIndexAsSection;

namespace fir {
class FirOpBuilder;
} // namespace fir
namespace language::Compability {

namespace semantics {
class Symbol;
} // namespace semantics

namespace parser {
struct OmpObject;
struct OmpObjectList;
} // namespace parser

namespace lower {
class StatementContext;
namespace pft {
struct Evaluation;
}

class AbstractConverter;

namespace omp {

struct DeclareTargetCaptureInfo {
  mlir::omp::DeclareTargetCaptureClause clause;
  bool automap = false;
  const semantics::Symbol &symbol;

  DeclareTargetCaptureInfo(mlir::omp::DeclareTargetCaptureClause c,
                           const semantics::Symbol &s, bool a = false)
      : clause(c), automap(a), symbol(s) {}
};

// A small helper structure for keeping track of a component members MapInfoOp
// and index data when lowering OpenMP map clauses. Keeps track of the
// placement of the component in the derived type hierarchy it rests within,
// alongside the generated mlir::omp::MapInfoOp for the mapped component.
//
// As an example of what the contents of this data structure may be like,
// when provided the following derived type and map of that type:
//
// type :: bottom_layer
//   real(8) :: i2
//   real(4) :: array_i2(10)
//   real(4) :: array_j2(10)
// end type bottom_layer
//
// type :: top_layer
//   real(4) :: i
//   integer(4) :: array_i(10)
//   real(4) :: j
//   type(bottom_layer) :: nested
//   integer, allocatable :: array_j(:)
//   integer(4) :: k
// end type top_layer
//
// type(top_layer) :: top_dtype
//
// map(tofrom: top_dtype%nested%i2, top_dtype%k, top_dtype%nested%array_i2)
//
// We would end up with an OmpMapParentAndMemberData populated like below:
//
// memberPlacementIndices:
//  Vector 1: 3, 0
//  Vector 2: 5
//  Vector 3: 3, 1
//
// memberMap:
// Entry 1: omp.map.info for "top_dtype%nested%i2"
// Entry 2: omp.map.info for "top_dtype%k"
// Entry 3: omp.map.info for "top_dtype%nested%array_i2"
//
// And this OmpMapParentAndMemberData would be accessed via the parent
// symbol for top_dtype. Other parent derived type instances that have
// members mapped would have there own OmpMapParentAndMemberData entry
// accessed via their own symbol.
struct OmpMapParentAndMemberData {
  // The indices representing the component members placement in its derived
  // type parents hierarchy.
  toolchain::SmallVector<toolchain::SmallVector<int64_t>> memberPlacementIndices;

  // Placement of the member in the member vector.
  toolchain::SmallVector<mlir::omp::MapInfoOp> memberMap;

  bool isDuplicateMemberMapInfo(toolchain::SmallVectorImpl<int64_t> &memberIndices) {
    return toolchain::find_if(memberPlacementIndices, [&](auto &memberData) {
             return toolchain::equal(memberIndices, memberData);
           }) != memberPlacementIndices.end();
  }

  void addChildIndexAndMapToParent(const omp::Object &object,
                                   mlir::omp::MapInfoOp &mapOp,
                                   semantics::SemanticsContext &semaCtx);
};

mlir::omp::MapInfoOp
createMapInfoOp(fir::FirOpBuilder &builder, mlir::Location loc,
                mlir::Value baseAddr, mlir::Value varPtrPtr,
                toolchain::StringRef name, toolchain::ArrayRef<mlir::Value> bounds,
                toolchain::ArrayRef<mlir::Value> members,
                mlir::ArrayAttr membersIndex, uint64_t mapType,
                mlir::omp::VariableCaptureKind mapCaptureType, mlir::Type retTy,
                bool partialMap = false,
                mlir::FlatSymbolRefAttr mapperId = mlir::FlatSymbolRefAttr());

void insertChildMapInfoIntoParent(
    language::Compability::lower::AbstractConverter &converter,
    language::Compability::semantics::SemanticsContext &semaCtx,
    language::Compability::lower::StatementContext &stmtCtx,
    std::map<Object, OmpMapParentAndMemberData> &parentMemberIndices,
    toolchain::SmallVectorImpl<mlir::Value> &mapOperands,
    toolchain::SmallVectorImpl<const semantics::Symbol *> &mapSyms);

void generateMemberPlacementIndices(
    const Object &object, toolchain::SmallVectorImpl<int64_t> &indices,
    language::Compability::semantics::SemanticsContext &semaCtx);

bool isMemberOrParentAllocatableOrPointer(
    const Object &object, language::Compability::semantics::SemanticsContext &semaCtx);

mlir::Value createParentSymAndGenIntermediateMaps(
    mlir::Location clauseLocation, language::Compability::lower::AbstractConverter &converter,
    semantics::SemanticsContext &semaCtx, lower::StatementContext &stmtCtx,
    omp::ObjectList &objectList, toolchain::SmallVectorImpl<int64_t> &indices,
    OmpMapParentAndMemberData &parentMemberIndices, toolchain::StringRef asFortran,
    toolchain::omp::OpenMPOffloadMappingFlags mapTypeBits);

omp::ObjectList gatherObjectsOf(omp::Object derivedTypeMember,
                                semantics::SemanticsContext &semaCtx);

mlir::Type getLoopVarType(lower::AbstractConverter &converter,
                          std::size_t loopVarTypeSize);

semantics::Symbol *
getIterationVariableSymbol(const lower::pft::Evaluation &eval);

void gatherFuncAndVarSyms(
    const ObjectList &objects, mlir::omp::DeclareTargetCaptureClause clause,
    toolchain::SmallVectorImpl<DeclareTargetCaptureInfo> &symbolAndClause,
    bool automap = false);

int64_t getCollapseValue(const List<Clause> &clauses);

void genObjectList(const ObjectList &objects,
                   lower::AbstractConverter &converter,
                   toolchain::SmallVectorImpl<mlir::Value> &operands);

void lastprivateModifierNotSupported(const omp::clause::Lastprivate &lastp,
                                     mlir::Location loc);

bool collectLoopRelatedInfo(
    lower::AbstractConverter &converter, mlir::Location currentLocation,
    lower::pft::Evaluation &eval, const omp::List<omp::Clause> &clauses,
    mlir::omp::LoopRelatedClauseOps &result,
    toolchain::SmallVectorImpl<const semantics::Symbol *> &iv);

} // namespace omp
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_OPENMPUTILS_H
