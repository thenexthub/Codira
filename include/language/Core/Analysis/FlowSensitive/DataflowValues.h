//===--- DataflowValues.h - Data structure for dataflow values --*- C++ -*-===//
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
// This file defines a skeleton data structure for encapsulating the dataflow
// values for a CFG.  Typically this is subclassed to provide methods for
// computing these values from a CFG.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSES_DATAFLOW_VALUES
#define LANGUAGE_CORE_ANALYSES_DATAFLOW_VALUES

#include "language/Core/Analysis/CFG.h"
#include "language/Core/Analysis/ProgramPoint.h"
#include "toolchain/ADT/DenseMap.h"

namespace language::Core {

//===----------------------------------------------------------------------===//
/// Dataflow Directional Tag Classes.  These are used for tag dispatching
///  within the dataflow solver/transfer functions to determine what direction
///  a dataflow analysis flows.
//===----------------------------------------------------------------------===//

namespace dataflow {
  struct forward_analysis_tag {};
  struct backward_analysis_tag {};
} // end namespace dataflow

//===----------------------------------------------------------------------===//
/// DataflowValues.  Container class to store dataflow values for a CFG.
//===----------------------------------------------------------------------===//

template <typename ValueTypes,
          typename _AnalysisDirTag = dataflow::forward_analysis_tag >
class DataflowValues {

  //===--------------------------------------------------------------------===//
  // Type declarations.
  //===--------------------------------------------------------------------===//

public:
  using ValTy = typename ValueTypes::ValTy;
  using AnalysisDataTy = typename ValueTypes::AnalysisDataTy;
  using AnalysisDirTag = _AnalysisDirTag;
  using EdgeDataMapTy = toolchain::DenseMap<ProgramPoint, ValTy>;
  using BlockDataMapTy = toolchain::DenseMap<const CFGBlock *, ValTy>;
  using StmtDataMapTy = toolchain::DenseMap<const Stmt *, ValTy>;

  //===--------------------------------------------------------------------===//
  // Predicates.
  //===--------------------------------------------------------------------===//

public:
  /// isForwardAnalysis - Returns true if the dataflow values are computed
  ///  from a forward analysis.
  bool isForwardAnalysis() { return isForwardAnalysis(AnalysisDirTag()); }

  /// isBackwardAnalysis - Returns true if the dataflow values are computed
  ///  from a backward analysis.
  bool isBackwardAnalysis() { return !isForwardAnalysis(); }

private:
  bool isForwardAnalysis(dataflow::forward_analysis_tag)  { return true; }
  bool isForwardAnalysis(dataflow::backward_analysis_tag) { return false; }

  //===--------------------------------------------------------------------===//
  // Initialization and accessors methods.
  //===--------------------------------------------------------------------===//

public:
  DataflowValues() : StmtDataMap(NULL) {}
  ~DataflowValues() { delete StmtDataMap; }

  /// InitializeValues - Invoked by the solver to initialize state needed for
  ///  dataflow analysis.  This method is usually specialized by subclasses.
  void InitializeValues(const CFG& cfg) {}


  /// getEdgeData - Retrieves the dataflow values associated with a
  ///  CFG edge.
  ValTy& getEdgeData(const BlockEdge &E) {
    typename EdgeDataMapTy::iterator I = EdgeDataMap.find(E);
    assert (I != EdgeDataMap.end() && "No data associated with Edge.");
    return I->second;
  }

  const ValTy& getEdgeData(const BlockEdge &E) const {
    return reinterpret_cast<DataflowValues*>(this)->getEdgeData(E);
  }

  /// getBlockData - Retrieves the dataflow values associated with a
  ///  specified CFGBlock.  If the dataflow analysis is a forward analysis,
  ///  this data is associated with the END of the block.  If the analysis
  ///  is a backwards analysis, it is associated with the ENTRY of the block.
  ValTy& getBlockData(const CFGBlock *B) {
    typename BlockDataMapTy::iterator I = BlockDataMap.find(B);
    assert (I != BlockDataMap.end() && "No data associated with block.");
    return I->second;
  }

  const ValTy& getBlockData(const CFGBlock *B) const {
    return const_cast<DataflowValues*>(this)->getBlockData(B);
  }

  /// getStmtData - Retrieves the dataflow values associated with a
  ///  specified Stmt.  If the dataflow analysis is a forward analysis,
  ///  this data corresponds to the point immediately before a Stmt.
  ///  If the analysis is a backwards analysis, it is associated with
  ///  the point after a Stmt.  This data is only computed for block-level
  ///  expressions, and only when requested when the analysis is executed.
  ValTy& getStmtData(const Stmt *S) {
    assert (StmtDataMap && "Dataflow values were not computed for statements.");
    typename StmtDataMapTy::iterator I = StmtDataMap->find(S);
    assert (I != StmtDataMap->end() && "No data associated with statement.");
    return I->second;
  }

  const ValTy& getStmtData(const Stmt *S) const {
    return const_cast<DataflowValues*>(this)->getStmtData(S);
  }

  /// getEdgeDataMap - Retrieves the internal map between CFG edges and
  ///  dataflow values.  Usually used by a dataflow solver to compute
  ///  values for blocks.
  EdgeDataMapTy& getEdgeDataMap() { return EdgeDataMap; }
  const EdgeDataMapTy& getEdgeDataMap() const { return EdgeDataMap; }

  /// getBlockDataMap - Retrieves the internal map between CFGBlocks and
  /// dataflow values.  If the dataflow analysis operates in the forward
  /// direction, the values correspond to the dataflow values at the start
  /// of the block.  Otherwise, for a backward analysis, the values correspond
  /// to the dataflow values at the end of the block.
  BlockDataMapTy& getBlockDataMap() { return BlockDataMap; }
  const BlockDataMapTy& getBlockDataMap() const { return BlockDataMap; }

  /// getStmtDataMap - Retrieves the internal map between Stmts and
  /// dataflow values.
  StmtDataMapTy& getStmtDataMap() {
    if (!StmtDataMap) StmtDataMap = new StmtDataMapTy();
    return *StmtDataMap;
  }

  const StmtDataMapTy& getStmtDataMap() const {
    return const_cast<DataflowValues*>(this)->getStmtDataMap();
  }

  /// getAnalysisData - Retrieves the meta data associated with a
  ///  dataflow analysis for analyzing a particular CFG.
  ///  This is typically consumed by transfer function code (via the solver).
  ///  This can also be used by subclasses to interpret the dataflow values.
  AnalysisDataTy& getAnalysisData() { return AnalysisData; }
  const AnalysisDataTy& getAnalysisData() const { return AnalysisData; }

  //===--------------------------------------------------------------------===//
  // Internal data.
  //===--------------------------------------------------------------------===//

protected:
  EdgeDataMapTy      EdgeDataMap;
  BlockDataMapTy     BlockDataMap;
  StmtDataMapTy*     StmtDataMap;
  AnalysisDataTy     AnalysisData;
};

} // end namespace language::Core
#endif // LANGUAGE_CORE_ANALYSES_DATAFLOW_VALUES
