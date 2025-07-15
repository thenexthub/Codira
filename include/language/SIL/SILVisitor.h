//===--- SILVisitor.h - Defines the SILVisitor class ------------*- C++ -*-===//
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
// This file defines the SILVisitor class, used for walking SIL code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILVISITOR_H
#define LANGUAGE_SIL_SILVISITOR_H

#include "language/SIL/SILFunction.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILUndef.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language {

/// A helper class for all the SIL visitors.
/// You probably shouldn't use this directly.
template <typename ImplClass, typename RetTy = void, typename... ArgTys>
class SILVisitorBase {
public:
  ImplClass &asImpl() { return static_cast<ImplClass &>(*this); }

  void visitSILBasicBlock(SILBasicBlock *BB, ArgTys... args) {
    asImpl().visitBasicBlockArguments(BB, args...);

    for (auto &I : *BB)
      asImpl().visit(&I, args...);
  }
  void visitSILBasicBlock(SILBasicBlock &BB, ArgTys... args) {
    asImpl().visitSILBasicBlock(&BB, args...);
  }

  void visitSILFunction(SILFunction *F, ArgTys... args) {
    for (auto &BB : *F)
      asImpl().visitSILBasicBlock(&BB, args...);
  }
  void visitSILFunction(SILFunction &F, ArgTys... args) {
    asImpl().visitSILFunction(&F, args...);
  }
};

/// SILValueVisitor - This is a simple visitor class for Codira SIL nodes,
/// allowing clients to walk over entire SIL functions, blocks, or instructions.
template <typename ImplClass, typename RetTy = void, typename... ArgTys>
class SILValueVisitor
       : public SILVisitorBase<ImplClass, RetTy, ArgTys...> {
  using super = SILVisitorBase<ImplClass, RetTy, ArgTys...>;
public:
  using super::asImpl;

  RetTy visit(ValueBase *V, ArgTys... args) {
    switch (V->getKind()) {
#define VALUE(CLASS, PARENT)                                \
  case ValueKind::CLASS:                                    \
    return asImpl().visit##CLASS(static_cast<CLASS*>(V),    \
                                 std::forward<ArgTys>(args)...);
#include "language/SIL/SILNodes.def"
    }
    toolchain_unreachable("Not reachable, all cases handled");
  }

  // Define default dispatcher implementations chain to parent nodes.
#define VALUE(CLASS, PARENT)                                         \
  RetTy visit##CLASS(CLASS *I, ArgTys... args) {                     \
    return asImpl().visit##PARENT(I, std::forward<ArgTys>(args)...); \
  }
#define ABSTRACT_VALUE(CLASS, PARENT) VALUE(CLASS, PARENT)
#include "language/SIL/SILNodes.def"
};

/// A visitor that should only visit SIL instructions.
template <typename ImplClass, typename RetTy = void, typename... ArgTys>
class SILInstructionVisitor
       : public SILVisitorBase<ImplClass, RetTy, ArgTys...> {
  using super = SILVisitorBase<ImplClass, RetTy, ArgTys...>;
public:
  using super::asImpl;

  // Perform any required pre-processing before visiting.
  // Sub-classes can override it to provide their custom
  // pre-processing steps.
  void beforeVisit(SILInstruction *inst) {}

  RetTy visit(SILInstruction *inst, ArgTys... args) {
    asImpl().beforeVisit(inst, args...);

    switch (inst->getKind()) {
#define INST(CLASS, PARENT)                                             \
    case SILInstructionKind::CLASS:                                     \
      return asImpl().visit##CLASS(static_cast<CLASS*>(inst),           \
                                   std::forward<ArgTys>(args)...);
#include "language/SIL/SILNodes.def"
    }
    toolchain_unreachable("Not reachable, all cases handled");
  }

  // Define default dispatcher implementations chain to parent nodes.
#define INST(CLASS, PARENT)                                             \
  RetTy visit##CLASS(CLASS *inst, ArgTys... args) {                     \
    return asImpl().visit##PARENT(inst, std::forward<ArgTys>(args)...); \
  }
#define ABSTRACT_INST(CLASS, PARENT) INST(CLASS, PARENT)
#include "language/SIL/SILNodes.def"

  void visitBasicBlockArguments(SILBasicBlock *BB, ArgTys... args) {}
};

/// A visitor that should visit all SIL nodes.
template <typename ImplClass, typename RetTy = void, typename... ArgTys>
class SILNodeVisitor
       : public SILVisitorBase<ImplClass, RetTy, ArgTys...> {
  using super = SILVisitorBase<ImplClass, RetTy, ArgTys...>;
public:
  using super::asImpl;

  // Perform any required pre-processing before visiting.
  // Sub-classes can override it to provide their custom
  // pre-processing steps.
  void beforeVisit(SILNode *I, ArgTys... args) {}

  RetTy visit(SILNode *node, ArgTys... args) {
    asImpl().beforeVisit(node, args...);

    switch (node->getKind()) {
#define NODE(CLASS, PARENT)                                             \
    case SILNodeKind::CLASS:                                            \
      return asImpl().visit##CLASS(cast<CLASS>(node),                   \
                                   std::forward<ArgTys>(args)...);
#include "language/SIL/SILNodes.def"
    }
    toolchain_unreachable("Not reachable, all cases handled");
  }

  // Define default dispatcher implementations chain to parent nodes.
#define NODE(CLASS, PARENT)                                             \
  RetTy visit##CLASS(CLASS *node, ArgTys... args) {                     \
    return asImpl().visit##PARENT(node, std::forward<ArgTys>(args)...); \
  }
#define ABSTRACT_NODE(CLASS, PARENT) NODE(CLASS, PARENT)
#include "language/SIL/SILNodes.def"

  void visitBasicBlockArguments(SILBasicBlock *BB, ArgTys... args) {
    for (auto argI = BB->args_begin(), argEnd = BB->args_end(); argI != argEnd;
         ++argI)
      asImpl().visit(*argI, args...);
  }
};

} // end namespace language

#endif
