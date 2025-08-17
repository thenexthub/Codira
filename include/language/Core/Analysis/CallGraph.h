//===- CallGraph.h - AST-based Call graph -----------------------*- C++ -*-===//
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
//  This file declares the AST-based CallGraph.
//
//  A call graph for functions whose definitions/bodies are available in the
//  current translation unit. The graph has a "virtual" root node that contains
//  edges to all externally available functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_CALLGRAPH_H
#define LANGUAGE_CORE_ANALYSIS_CALLGRAPH_H

#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/AST/DynamicRecursiveASTVisitor.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/GraphTraits.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/iterator_range.h"
#include <memory>

namespace language::Core {

class CallGraphNode;
class Decl;
class DeclContext;
class Stmt;

/// The AST-based call graph.
///
/// The call graph extends itself with the given declarations by implementing
/// the recursive AST visitor, which constructs the graph by visiting the given
/// declarations.
class CallGraph : public DynamicRecursiveASTVisitor {
  friend class CallGraphNode;

  using FunctionMapTy =
      toolchain::DenseMap<const Decl *, std::unique_ptr<CallGraphNode>>;

  /// FunctionMap owns all CallGraphNodes.
  FunctionMapTy FunctionMap;

  /// This is a virtual root node that has edges to all the functions.
  CallGraphNode *Root;

public:
  CallGraph();
  ~CallGraph();

  /// Populate the call graph with the functions in the given
  /// declaration.
  ///
  /// Recursively walks the declaration to find all the dependent Decls as well.
  void addToCallGraph(Decl *D) {
    TraverseDecl(D);
  }

  /// Determine if a declaration should be included in the graph.
  static bool includeInGraph(const Decl *D);

  /// Determine if a declaration should be included in the graph for the
  /// purposes of being a callee. This is similar to includeInGraph except
  /// it permits declarations, not just definitions.
  static bool includeCalleeInGraph(const Decl *D);

  /// Lookup the node for the given declaration.
  CallGraphNode *getNode(const Decl *) const;

  /// Lookup the node for the given declaration. If none found, insert
  /// one into the graph.
  CallGraphNode *getOrInsertNode(Decl *);

  using iterator = FunctionMapTy::iterator;
  using const_iterator = FunctionMapTy::const_iterator;

  /// Iterators through all the elements in the graph. Note, this gives
  /// non-deterministic order.
  iterator begin() { return FunctionMap.begin(); }
  iterator end()   { return FunctionMap.end();   }
  const_iterator begin() const { return FunctionMap.begin(); }
  const_iterator end()   const { return FunctionMap.end();   }

  /// Get the number of nodes in the graph.
  unsigned size() const { return FunctionMap.size(); }

  /// Get the virtual root of the graph, all the functions available externally
  /// are represented as callees of the node.
  CallGraphNode *getRoot() const { return Root; }

  /// Iterators through all the nodes of the graph that have no parent. These
  /// are the unreachable nodes, which are either unused or are due to us
  /// failing to add a call edge due to the analysis imprecision.
  using nodes_iterator = toolchain::SetVector<CallGraphNode *>::iterator;
  using const_nodes_iterator = toolchain::SetVector<CallGraphNode *>::const_iterator;

  void print(raw_ostream &os) const;
  void dump() const;
  void viewGraph() const;

  void addNodesForBlocks(DeclContext *D);

  /// Part of recursive declaration visitation. We recursively visit all the
  /// declarations to collect the root functions.
  bool VisitFunctionDecl(FunctionDecl *FD) override {
    // We skip function template definitions, as their semantics is
    // only determined when they are instantiated.
    if (includeInGraph(FD) && FD->isThisDeclarationADefinition()) {
      // Add all blocks declared inside this function to the graph.
      addNodesForBlocks(FD);
      // If this function has external linkage, anything could call it.
      // Note, we are not precise here. For example, the function could have
      // its address taken.
      addNodeForDecl(FD, FD->isGlobal());
    }
    return true;
  }

  /// Part of recursive declaration visitation.
  bool VisitObjCMethodDecl(ObjCMethodDecl *MD) override {
    if (includeInGraph(MD)) {
      addNodesForBlocks(MD);
      addNodeForDecl(MD, true);
    }
    return true;
  }

  // We are only collecting the declarations, so do not step into the bodies.
  bool TraverseStmt(Stmt *S) override { return true; }

private:
  /// Add the given declaration to the call graph.
  void addNodeForDecl(Decl *D, bool IsGlobal);
};

class CallGraphNode {
public:
  struct CallRecord {
    CallGraphNode *Callee;
    Expr *CallExpr;

    CallRecord() = default;

    CallRecord(CallGraphNode *Callee_, Expr *CallExpr_)
        : Callee(Callee_), CallExpr(CallExpr_) {}

    // The call destination is the only important data here,
    // allow to transparently unwrap into it.
    operator CallGraphNode *() const { return Callee; }
  };

private:
  /// The function/method declaration.
  Decl *FD;

  /// The list of functions called from this node.
  SmallVector<CallRecord, 5> CalledFunctions;

public:
  CallGraphNode(Decl *D) : FD(D) {}

  using iterator = SmallVectorImpl<CallRecord>::iterator;
  using const_iterator = SmallVectorImpl<CallRecord>::const_iterator;

  /// Iterators through all the callees/children of the node.
  iterator begin() { return CalledFunctions.begin(); }
  iterator end() { return CalledFunctions.end(); }
  const_iterator begin() const { return CalledFunctions.begin(); }
  const_iterator end() const { return CalledFunctions.end(); }

  /// Iterator access to callees/children of the node.
  toolchain::iterator_range<iterator> callees() {
    return toolchain::make_range(begin(), end());
  }
  toolchain::iterator_range<const_iterator> callees() const {
    return toolchain::make_range(begin(), end());
  }

  bool empty() const { return CalledFunctions.empty(); }
  unsigned size() const { return CalledFunctions.size(); }

  void addCallee(CallRecord Call) { CalledFunctions.push_back(Call); }

  Decl *getDecl() const { return FD; }

  FunctionDecl *getDefinition() const {
    return getDecl()->getAsFunction()->getDefinition();
  }

  void print(raw_ostream &os) const;
  void dump() const;
};

// NOTE: we are comparing based on the callee only. So different call records
// (with different call expressions) to the same callee will compare equal!
inline bool operator==(const CallGraphNode::CallRecord &LHS,
                       const CallGraphNode::CallRecord &RHS) {
  return LHS.Callee == RHS.Callee;
}

} // namespace language::Core

namespace toolchain {

// Specialize DenseMapInfo for language::Core::CallGraphNode::CallRecord.
template <> struct DenseMapInfo<language::Core::CallGraphNode::CallRecord> {
  static inline language::Core::CallGraphNode::CallRecord getEmptyKey() {
    return language::Core::CallGraphNode::CallRecord(
        DenseMapInfo<language::Core::CallGraphNode *>::getEmptyKey(),
        DenseMapInfo<language::Core::Expr *>::getEmptyKey());
  }

  static inline language::Core::CallGraphNode::CallRecord getTombstoneKey() {
    return language::Core::CallGraphNode::CallRecord(
        DenseMapInfo<language::Core::CallGraphNode *>::getTombstoneKey(),
        DenseMapInfo<language::Core::Expr *>::getTombstoneKey());
  }

  static unsigned getHashValue(const language::Core::CallGraphNode::CallRecord &Val) {
    // NOTE: we are comparing based on the callee only.
    // Different call records with the same callee will compare equal!
    return DenseMapInfo<language::Core::CallGraphNode *>::getHashValue(Val.Callee);
  }

  static bool isEqual(const language::Core::CallGraphNode::CallRecord &LHS,
                      const language::Core::CallGraphNode::CallRecord &RHS) {
    return LHS == RHS;
  }
};

// Graph traits for iteration, viewing.
template <> struct GraphTraits<language::Core::CallGraphNode*> {
  using NodeType = language::Core::CallGraphNode;
  using NodeRef = language::Core::CallGraphNode *;
  using ChildIteratorType = NodeType::iterator;

  static NodeType *getEntryNode(language::Core::CallGraphNode *CGN) { return CGN; }
  static ChildIteratorType child_begin(NodeType *N) { return N->begin();  }
  static ChildIteratorType child_end(NodeType *N) { return N->end(); }
};

template <> struct GraphTraits<const language::Core::CallGraphNode*> {
  using NodeType = const language::Core::CallGraphNode;
  using NodeRef = const language::Core::CallGraphNode *;
  using ChildIteratorType = NodeType::const_iterator;

  static NodeType *getEntryNode(const language::Core::CallGraphNode *CGN) { return CGN; }
  static ChildIteratorType child_begin(NodeType *N) { return N->begin();}
  static ChildIteratorType child_end(NodeType *N) { return N->end(); }
};

template <> struct GraphTraits<language::Core::CallGraph*>
  : public GraphTraits<language::Core::CallGraphNode*> {
  static NodeType *getEntryNode(language::Core::CallGraph *CGN) {
    return CGN->getRoot();  // Start at the external node!
  }

  static language::Core::CallGraphNode *
  CGGetValue(language::Core::CallGraph::const_iterator::value_type &P) {
    return P.second.get();
  }

  // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
  using nodes_iterator =
      mapped_iterator<language::Core::CallGraph::iterator, decltype(&CGGetValue)>;

  static nodes_iterator nodes_begin(language::Core::CallGraph *CG) {
    return nodes_iterator(CG->begin(), &CGGetValue);
  }

  static nodes_iterator nodes_end  (language::Core::CallGraph *CG) {
    return nodes_iterator(CG->end(), &CGGetValue);
  }

  static unsigned size(language::Core::CallGraph *CG) { return CG->size(); }
};

template <> struct GraphTraits<const language::Core::CallGraph*> :
  public GraphTraits<const language::Core::CallGraphNode*> {
  static NodeType *getEntryNode(const language::Core::CallGraph *CGN) {
    return CGN->getRoot();
  }

  static language::Core::CallGraphNode *
  CGGetValue(language::Core::CallGraph::const_iterator::value_type &P) {
    return P.second.get();
  }

  // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
  using nodes_iterator =
      mapped_iterator<language::Core::CallGraph::const_iterator, decltype(&CGGetValue)>;

  static nodes_iterator nodes_begin(const language::Core::CallGraph *CG) {
    return nodes_iterator(CG->begin(), &CGGetValue);
  }

  static nodes_iterator nodes_end(const language::Core::CallGraph *CG) {
    return nodes_iterator(CG->end(), &CGGetValue);
  }

  static unsigned size(const language::Core::CallGraph *CG) { return CG->size(); }
};

} // namespace toolchain

#endif // LANGUAGE_CORE_ANALYSIS_CALLGRAPH_H
