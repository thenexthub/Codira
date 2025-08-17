//===-- MPIFunctionClassifier.h - classifies MPI functions ----*- C++ -*-===//
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
///
/// \file
/// This file defines functionality to identify and classify MPI functions.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CHECKERS_MPIFUNCTIONCLASSIFIER_H
#define LANGUAGE_CORE_STATICANALYZER_CHECKERS_MPIFUNCTIONCLASSIFIER_H

#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

namespace language::Core {
namespace ento {
namespace mpi {

class MPIFunctionClassifier {
public:
  MPIFunctionClassifier(ASTContext &ASTCtx) { identifierInit(ASTCtx); }

  // general identifiers
  bool isMPIType(const IdentifierInfo *const IdentInfo) const;
  bool isNonBlockingType(const IdentifierInfo *const IdentInfo) const;

  // point-to-point identifiers
  bool isPointToPointType(const IdentifierInfo *const IdentInfo) const;

  // collective identifiers
  bool isCollectiveType(const IdentifierInfo *const IdentInfo) const;
  bool isCollToColl(const IdentifierInfo *const IdentInfo) const;
  bool isScatterType(const IdentifierInfo *const IdentInfo) const;
  bool isGatherType(const IdentifierInfo *const IdentInfo) const;
  bool isAllgatherType(const IdentifierInfo *const IdentInfo) const;
  bool isAlltoallType(const IdentifierInfo *const IdentInfo) const;
  bool isReduceType(const IdentifierInfo *const IdentInfo) const;
  bool isBcastType(const IdentifierInfo *const IdentInfo) const;

  // additional identifiers
  bool isMPI_Wait(const IdentifierInfo *const IdentInfo) const;
  bool isMPI_Waitall(const IdentifierInfo *const IdentInfo) const;
  bool isWaitType(const IdentifierInfo *const IdentInfo) const;

private:
  // Initializes function identifiers, to recognize them during analysis.
  void identifierInit(ASTContext &ASTCtx);
  void initPointToPointIdentifiers(ASTContext &ASTCtx);
  void initCollectiveIdentifiers(ASTContext &ASTCtx);
  void initAdditionalIdentifiers(ASTContext &ASTCtx);

  // The containers are used, to enable classification of MPI-functions during
  // analysis.
  toolchain::SmallVector<IdentifierInfo *, 12> MPINonBlockingTypes;

  toolchain::SmallVector<IdentifierInfo *, 10> MPIPointToPointTypes;
  toolchain::SmallVector<IdentifierInfo *, 16> MPICollectiveTypes;

  toolchain::SmallVector<IdentifierInfo *, 4> MPIPointToCollTypes;
  toolchain::SmallVector<IdentifierInfo *, 4> MPICollToPointTypes;
  toolchain::SmallVector<IdentifierInfo *, 6> MPICollToCollTypes;

  toolchain::SmallVector<IdentifierInfo *, 32> MPIType;

  // point-to-point functions
  IdentifierInfo *IdentInfo_MPI_Send = nullptr, *IdentInfo_MPI_Isend = nullptr,
      *IdentInfo_MPI_Ssend = nullptr, *IdentInfo_MPI_Issend = nullptr,
      *IdentInfo_MPI_Bsend = nullptr, *IdentInfo_MPI_Ibsend = nullptr,
      *IdentInfo_MPI_Rsend = nullptr, *IdentInfo_MPI_Irsend = nullptr,
      *IdentInfo_MPI_Recv = nullptr, *IdentInfo_MPI_Irecv = nullptr;

  // collective functions
  IdentifierInfo *IdentInfo_MPI_Scatter = nullptr,
      *IdentInfo_MPI_Iscatter = nullptr, *IdentInfo_MPI_Gather = nullptr,
      *IdentInfo_MPI_Igather = nullptr, *IdentInfo_MPI_Allgather = nullptr,
      *IdentInfo_MPI_Iallgather = nullptr, *IdentInfo_MPI_Bcast = nullptr,
      *IdentInfo_MPI_Ibcast = nullptr, *IdentInfo_MPI_Reduce = nullptr,
      *IdentInfo_MPI_Ireduce = nullptr, *IdentInfo_MPI_Allreduce = nullptr,
      *IdentInfo_MPI_Iallreduce = nullptr, *IdentInfo_MPI_Alltoall = nullptr,
      *IdentInfo_MPI_Ialltoall = nullptr, *IdentInfo_MPI_Barrier = nullptr;

  // additional functions
  IdentifierInfo *IdentInfo_MPI_Comm_rank = nullptr,
      *IdentInfo_MPI_Comm_size = nullptr, *IdentInfo_MPI_Wait = nullptr,
      *IdentInfo_MPI_Waitall = nullptr;
};

} // end of namespace: mpi
} // end of namespace: ento
} // end of namespace: clang

#endif
