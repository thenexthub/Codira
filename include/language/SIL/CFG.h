//===--- CFG.h - SIL control-flow graph utilities ---------------*- C++ -*-===//
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
// This file provides basic declarations and utilities for working with
// SIL basic blocks as a control-flow graph.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_CFG_H
#define LANGUAGE_SIL_CFG_H

#include "language/SIL/SILFunction.h"
#include "language/SIL/SILValue.h"
#include "toolchain/ADT/GraphTraits.h"

#if defined(__has_include)
#if __has_include("toolchain/Support/CfgTraits.h")
#include "toolchain/Support/CfgTraits.h"
#define LANGUAGE_TOOLCHAIN_HAS_CFGTRAITS_H
#endif
#endif

namespace toolchain {

//===----------------------------------------------------------------------===//
// GraphTraits for SILBasicBlock
//===----------------------------------------------------------------------===//

template <> struct GraphTraits<language::SILBasicBlock *> {
  using ChildIteratorType = language::SILBasicBlock::succblock_iterator;
  using Node = language::SILBasicBlock;
  using NodeRef = Node *;

  static NodeRef getEntryNode(NodeRef BB) { return BB; }

  static ChildIteratorType child_begin(NodeRef N) {
    return N->succblock_begin();
  }
  static ChildIteratorType child_end(NodeRef N) { return N->succblock_end(); }
};

template <> struct GraphTraits<const language::SILBasicBlock*> {
  using ChildIteratorType = language::SILBasicBlock::const_succblock_iterator;
  using Node = const language::SILBasicBlock;
  using NodeRef = Node *;

  static NodeRef getEntryNode(NodeRef BB) { return BB; }

  static ChildIteratorType child_begin(NodeRef N) {
    return N->succblock_begin();
  }
  static ChildIteratorType child_end(NodeRef N) { return N->succblock_end(); }
};

template <> struct GraphTraits<Inverse<language::SILBasicBlock*> > {
  using ChildIteratorType = language::SILBasicBlock::pred_iterator;
  using Node = language::SILBasicBlock;
  using NodeRef = Node *;
  static NodeRef getEntryNode(Inverse<language::SILBasicBlock *> G) {
    return G.Graph;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->pred_begin();
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return N->pred_end();
  }
};

template <> struct GraphTraits<Inverse<const language::SILBasicBlock*> > {
  using ChildIteratorType = language::SILBasicBlock::pred_iterator;
  using Node = const language::SILBasicBlock;
  using NodeRef = Node *;

  static NodeRef getEntryNode(Inverse<const language::SILBasicBlock *> G) {
    return G.Graph;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->pred_begin();
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return N->pred_end();
  }
};

template <>
struct GraphTraits<language::SILFunction *>
    : public GraphTraits<language::SILBasicBlock *> {
  using GraphType = language::SILFunction *;
  using NodeRef = language::SILBasicBlock *;

  static NodeRef getEntryNode(GraphType F) { return &F->front(); }

  using nodes_iterator = pointer_iterator<language::SILFunction::iterator>;
  static nodes_iterator nodes_begin(GraphType F) {
    return nodes_iterator(F->begin());
  }
  static nodes_iterator nodes_end(GraphType F) {
    return nodes_iterator(F->end());
  }
  static unsigned size(GraphType F) { return F->size(); }
};

template <> struct GraphTraits<Inverse<language::SILFunction*> >
    : public GraphTraits<Inverse<language::SILBasicBlock*> > {
  using GraphType = Inverse<language::SILFunction *>;
  using NodeRef = NodeRef;

  static NodeRef getEntryNode(GraphType F) { return &F.Graph->front(); }

  using nodes_iterator = pointer_iterator<language::SILFunction::iterator>;
  static nodes_iterator nodes_begin(GraphType F) {
    return nodes_iterator(F.Graph->begin());
  }
  static nodes_iterator nodes_end(GraphType F) {
    return nodes_iterator(F.Graph->end());
  }
  static unsigned size(GraphType F) { return F.Graph->size(); }
};

#ifdef LANGUAGE_TOOLCHAIN_HAS_CFGTRAITS_H

class SILCfgTraitsBase : public CfgTraitsBase {
public:
  using ParentType = language::SILFunction;
  using BlockRef = language::SILBasicBlock *;
  using ValueRef = language::SILInstruction *;

  static CfgBlockRef wrapRef(BlockRef block) {
    return makeOpaque<CfgBlockRefTag>(block);
  }
  static CfgValueRef wrapRef(ValueRef block) {
    return makeOpaque<CfgValueRefTag>(block);
  }
  static BlockRef unwrapRef(CfgBlockRef block) {
    return static_cast<BlockRef>(getOpaque(block));
  }
  static ValueRef unwrapRef(CfgValueRef block) {
    return static_cast<ValueRef>(getOpaque(block));
  }
};
/// \brief CFG traits for SIL IR.
class SILCfgTraits : public CfgTraits<SILCfgTraitsBase, SILCfgTraits> {
public:
  explicit SILCfgTraits(language::SILFunction * /*parent*/) {}

  static language::SILFunction *getBlockParent(language::SILBasicBlock *block) {
    return block->getParent();
  }

  static auto predecessors(language::SILBasicBlock *block) {
    return block->getPredecessorBlocks();
  }
  static auto successors(language::SILBasicBlock *block) {
    return block->getSuccessors();
  }

  /// Get the defining block of a value if it is an instruction, or null
  /// otherwise.
  static BlockRef getValueDefBlock(ValueRef value) {
    if (auto *instruction = dyn_cast<language::SILInstruction>(value))
      return instruction->getParent();
    return nullptr;
  }

  struct block_iterator
      : iterator_adaptor_base<block_iterator, language::SILFunction::iterator> {
    using Base = iterator_adaptor_base<block_iterator, language::SILFunction::iterator>;

    block_iterator() = default;

    explicit block_iterator(language::SILFunction::iterator i) : Base(i) {}

    language::SILBasicBlock *operator*() const { return &Base::operator*(); }
  };

  static iterator_range<block_iterator> blocks(language::SILFunction *function) {
    return {block_iterator(function->begin()), block_iterator(function->end())};
  }

  struct value_iterator
      : iterator_adaptor_base<value_iterator, language::SILBasicBlock::iterator> {
    using Base = iterator_adaptor_base<value_iterator, language::SILBasicBlock::iterator>;

    value_iterator() = default;

    explicit value_iterator(language::SILBasicBlock::iterator i) : Base(i) {}

    ValueRef operator*() const { return &Base::operator*(); }
  };

  static iterator_range<value_iterator> blockdefs(BlockRef block) {
    return {value_iterator(block->begin()), value_iterator(block->end())};
  }
  struct Printer {
    explicit Printer(const SILCfgTraits &) {}
    ~Printer(){}

    void printBlockName(raw_ostream &out, BlockRef block) const {
      block->printAsOperand(out);
    }
    void printValue(raw_ostream &out, ValueRef value) const {
      value->print(out);
    }
  };
};

template <> struct CfgTraitsFor<language::SILBasicBlock> {
  using CfgTraits = SILCfgTraits;
};

#endif

} // end toolchain namespace

#endif

