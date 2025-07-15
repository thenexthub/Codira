//===--- SILBasicBlock.h - Basic blocks for SIL -----------------*- C++ -*-===//
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
// This file defines the high-level BasicBlocks used for Codira SIL code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_BASICBLOCK_H
#define LANGUAGE_SIL_BASICBLOCK_H

#include "language/Basic/Compiler.h"
#include "language/Basic/Range.h"
#include "language/Basic/LanguageObjectHeader.h"
#include "language/SIL/SILArgumentArrayRef.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILArgument.h"
#include "toolchain/ADT/TinyPtrVector.h"

namespace language {

class SILFunction;
class SILArgument;
class SILPrintContext;

/// Instruction iterator which allows to "delete" instructions while iterating
/// over the instruction list.
///
/// Iteration with this iterator allows to delete the current, the next or any
/// instruction in the list while iterating.
/// This works because instruction deletion is deferred (for details see
/// `SILModule::scheduledForDeletion`) and removing an instruction from the list
/// keeps the prev/next pointers (see `SILInstructionListBase`).
template <typename IteratorBase>
class DeletableInstructionsIterator {
  using Self = DeletableInstructionsIterator<IteratorBase>;

  IteratorBase base;
  IteratorBase end;

public:
  using value_type = typename IteratorBase::value_type;
  using difference_type = ptrdiff_t;
  using pointer = value_type *;
  using iterator_category = std::forward_iterator_tag;

  DeletableInstructionsIterator(IteratorBase base, IteratorBase end)
  : base(base), end(end) {}

  value_type &operator*() const { return *base; }

  SILInstruction *operator->() const { return base.operator->(); }

  Self &operator++() {
    // If the current instruction is "deleted" (which means: removed from the
    // list), it's prev/next pointers still point to the next instruction which
    // is still in the list - or "deleted", too.
    ++base;
    // Skip over all deleted instructions. Eventually we reach an instruction
    // is still in the list (= not "deleted") or the end iterator.
    while (base != end && base->isDeleted()) {
      ++base;
    }
    return *this;
  }

  bool operator==(const Self &rhs) const { return base == rhs.base; }
  bool operator!=(const Self &rhs) const { return !(*this == rhs); }
};

class SILBasicBlock :
public toolchain::ilist_node<SILBasicBlock>, public SILAllocated<SILBasicBlock>,
public LanguageObjectHeader {
  friend class SILSuccessor;
  friend class SILFunction;
  friend class SILGlobalVariable;
  template <typename, unsigned> friend class BasicBlockData;
  template <class, class> friend class SILBitfield;

  static CodiraMetatype registeredMetatype;
  
  using CustomBitsType = uint32_t;
  
public:
  using InstListType = toolchain::iplist<SILInstruction>;
private:
  /// A backreference to the containing SILFunction.
  SILFunction *Parent;

  /// PredList - This is a list of all of the terminator operands that are
  /// branching to this block, forming the predecessor list.  This is
  /// automatically managed by the SILSuccessor class.
  SILSuccessor *PredList = nullptr;

  /// This is the list of basic block arguments for this block.
  /// A TinyPtrVector is the right choice, because ~98% of blocks have 0 or 1
  /// arguments.
  TinyPtrVector<SILArgument *> ArgumentList;

  /// The ordered set of instructions in the SILBasicBlock.
  InstListType InstList;

  /// Used by BasicBlockData to index the Data vector.
  ///
  /// A value of -1 means that the index is not initialized yet.
  int index = -1;

  /// Custom bits managed by BasicBlockBitfield.
  CustomBitsType customBits = 0;
  
  /// The BasicBlockBitfield ID of the last initialized bitfield in customBits.
  /// Example:
  ///
  ///                   Last initialized field:
  ///           lastInitializedBitfieldID == C.bitfieldID
  ///                              |
  ///                              V
  /// customBits:  <unused> EE DDD C BB AAA
  ///              31         ...         0
  ///
  /// -> AAA, BB and C are initialized,
  ///    DD and EEE are uninitialized
  ///
  /// See also: SILBitfield::bitfieldID, SILFunction::currentBitfieldID.
  uint64_t lastInitializedBitfieldID = 0;

  // Used by `BasicBlockBitfield`.
  unsigned getCustomBits() const { return customBits; }
  // Used by `BasicBlockBitfield`.
  void setCustomBits(unsigned value) { customBits = value; }

  friend struct toolchain::ilist_traits<SILBasicBlock>;

  SILBasicBlock();
  SILBasicBlock(SILFunction *parent);

  void operator=(const SILBasicBlock &) = delete;
  void operator delete(void *Ptr, size_t) = delete;

public:
  static void registerBridgedMetatype(CodiraMetatype metatype) {
    registeredMetatype = metatype;
  }

  ~SILBasicBlock();

  enum { numCustomBits = std::numeric_limits<CustomBitsType>::digits };

  constexpr static const uint64_t maxBitfieldID =
      std::numeric_limits<uint64_t>::max();

  /// Gets the ID (= index in the function's block list) of the block.
  ///
  /// Returns -1 if the block is not contained in a function.
  /// Warning: This function is slow. Therefore it should only be used for
  ///          debug output.
  int getDebugID() const;

  void setDebugName(toolchain::StringRef name);
  std::optional<toolchain::StringRef> getDebugName() const;

  SILFunction *getParent() { return Parent; }
  SILFunction *getFunction() { return getParent(); }
  const SILFunction *getParent() const { return Parent; }

  SILModule &getModule() const;

  /// This method unlinks 'self' from the containing SILFunction and deletes it.
  void eraseFromParent();

  /// Replaces usages of this block with 'undef's and then deletes it.
  void removeDeadBlock();

  /// Remove all instructions of a SILGlobalVariable's static initializer block.
  void clearStaticInitializerBlock(SILModule &module);

  //===--------------------------------------------------------------------===//
  // SILInstruction List Inspection and Manipulation
  //===--------------------------------------------------------------------===//

  using iterator = InstListType::iterator;
  using const_iterator = InstListType::const_iterator;
  using reverse_iterator = InstListType::reverse_iterator;
  using const_reverse_iterator = InstListType::const_reverse_iterator;

  void insert(iterator InsertPt, SILInstruction *I);
  void insert(SILInstruction *InsertPt, SILInstruction *I) {
    insert(InsertPt->getIterator(), I);
  }

  void push_back(SILInstruction *I);
  void push_front(SILInstruction *I);
  void erase(SILInstruction *I);
  void erase(SILInstruction *I, SILModule &module);
  static void moveInstruction(SILInstruction *inst, SILInstruction *beforeInst);
  void moveInstructionToFront(SILInstruction *inst);

  void eraseAllInstructions(SILModule &module);

  SILInstruction &back() { return InstList.back(); }
  const SILInstruction &back() const {
    return const_cast<SILBasicBlock *>(this)->back();
  }

  SILInstruction &front() { return InstList.front(); }
  const SILInstruction &front() const {
    return const_cast<SILBasicBlock *>(this)->front();
  }

  /// Transfer the instructions from Other to the end of this block.
  void spliceAtEnd(SILBasicBlock *Other) {
    InstList.splice(end(), Other->InstList);
  }

  void spliceAtBegin(SILBasicBlock *Other) {
    InstList.splice(begin(), Other->InstList);
  }

  bool empty() const { return InstList.empty(); }
  iterator begin() { return InstList.begin(); }
  iterator end() { return InstList.end(); }
  const_iterator begin() const { return InstList.begin(); }
  const_iterator end() const { return InstList.end(); }
  reverse_iterator rbegin() { return InstList.rbegin(); }
  reverse_iterator rend() { return InstList.rend(); }
  const_reverse_iterator rbegin() const { return InstList.rbegin(); }
  const_reverse_iterator rend() const { return InstList.rend(); }

  toolchain::iterator_range<iterator> getRangeStartingAtInst(SILInstruction *inst) {
    assert(inst->getParent() == this);
    return {inst->getIterator(), end()};
  }

  toolchain::iterator_range<iterator> getRangeEndingAtInst(SILInstruction *inst) {
    assert(inst->getParent() == this);
    return {begin(), inst->getIterator()};
  }

  toolchain::iterator_range<reverse_iterator>
  getReverseRangeStartingAtInst(SILInstruction *inst) {
    assert(inst->getParent() == this);
    return {inst->getReverseIterator(), rend()};
  }

  toolchain::iterator_range<reverse_iterator>
  getReverseRangeEndingAtInst(SILInstruction *inst) {
    assert(inst->getParent() == this);
    return {rbegin(), inst->getReverseIterator()};
  }

  toolchain::iterator_range<const_iterator>
  getRangeStartingAtInst(SILInstruction *inst) const {
    assert(inst->getParent() == this);
    return {inst->getIterator(), end()};
  }

  toolchain::iterator_range<const_iterator>
  getRangeEndingAtInst(SILInstruction *inst) const {
    assert(inst->getParent() == this);
    return {begin(), inst->getIterator()};
  }

  toolchain::iterator_range<const_reverse_iterator>
  getReverseRangeStartingAtInst(SILInstruction *inst) const {
    assert(inst->getParent() == this);
    return {inst->getReverseIterator(), rend()};
  }

  toolchain::iterator_range<const_reverse_iterator>
  getReverseRangeEndingAtInst(SILInstruction *inst) const {
    assert(inst->getParent() == this);
    return {rbegin(), inst->getReverseIterator()};
  }

  /// Allows deleting instructions while iterating over all instructions of the
  /// block.
  ///
  /// For details see `DeletableInstructionsIterator`.
  toolchain::iterator_range<DeletableInstructionsIterator<iterator>>
  deletableInstructions() { return {{begin(), end()}, {end(), end()}}; }

  /// Allows deleting instructions while iterating over all instructions of the
  /// block in reverse order.
  ///
  /// For details see `DeletableInstructionsIterator`.
  toolchain::iterator_range<DeletableInstructionsIterator<reverse_iterator>>
  reverseDeletableInstructions() { return {{rbegin(), rend()}, {rend(), rend()}}; }

  TermInst *getTerminator() {
    assert(!InstList.empty() && "Can't get successors for malformed block");
    return cast<TermInst>(&InstList.back());
  }

  const TermInst *getTerminator() const {
    return const_cast<SILBasicBlock *>(this)->getTerminator();
  }

  /// Splits a basic block into two at the specified instruction.
  ///
  /// Note that all the instructions BEFORE the specified iterator
  /// stay as part of the original basic block. The old basic block is left
  /// without a terminator.
  SILBasicBlock *split(iterator I);

  /// Moves the instruction to the iterator in this basic block.
  void moveTo(SILBasicBlock::iterator To, SILInstruction *I);

  //===--------------------------------------------------------------------===//
  // SILBasicBlock Argument List Inspection and Manipulation
  //===--------------------------------------------------------------------===//

  using arg_iterator = TinyPtrVector<SILArgument *>::iterator;
  using const_arg_iterator = TinyPtrVector<SILArgument *>::const_iterator;

  bool args_empty() const { return ArgumentList.empty(); }
  size_t args_size() const { return ArgumentList.size(); }
  arg_iterator args_begin() { return ArgumentList.begin(); }
  arg_iterator args_end() { return ArgumentList.end(); }
  const_arg_iterator args_begin() const { return ArgumentList.begin(); }
  const_arg_iterator args_end() const { return ArgumentList.end(); }

  /// Iterator over the PHI arguments of a basic block.
  /// Defines an implicit cast operator on the iterator, so that this iterator
  /// can be used in the SSAUpdaterImpl.
  template <typename PHIArgT = SILPhiArgument,
            typename IteratorT = arg_iterator>
  class phi_iterator_impl {
  private:
    IteratorT It;

  public:
    explicit phi_iterator_impl(IteratorT A) : It(A) {}
    phi_iterator_impl &operator++() { ++It; return *this; }

    operator PHIArgT *() { return cast<PHIArgT>(*It); }
    bool operator==(const phi_iterator_impl& x) const { return It == x.It; }
    bool operator!=(const phi_iterator_impl& x) const { return !operator==(x); }
  };
  typedef phi_iterator_impl<> phi_iterator;
  typedef phi_iterator_impl<const SILPhiArgument,
                            SILBasicBlock::const_arg_iterator>
      const_phi_iterator;

  inline iterator_range<phi_iterator> phis() {
    return make_range(phi_iterator(args_begin()), phi_iterator(args_end()));
  }
  inline iterator_range<const_phi_iterator> phis() const {
    return make_range(const_phi_iterator(args_begin()),
                      const_phi_iterator(args_end()));
  }

  ArrayRef<SILArgument *> getArguments() const { return ArgumentList; }

  /// Returns a transform array ref that performs toolchain::cast<NAME>
  /// each argument and then returns the downcasted value.
#define ARGUMENT(NAME, PARENT) NAME##ArrayRef get##NAME##s() const;
#include "language/SIL/SILNodes.def"

  unsigned getNumArguments() const { return ArgumentList.size(); }
  const SILArgument *getArgument(unsigned i) const { return ArgumentList[i]; }
  SILArgument *getArgument(unsigned i) { return ArgumentList[i]; }

  void cloneArgumentList(SILBasicBlock *Other);

  void moveArgumentList(SILBasicBlock *from);

  /// Erase a specific argument from the arg list.
  void eraseArgument(int Index);

  /// Allocate a new argument of type \p Ty and append it to the argument
  /// list. Optionally you can pass in a value decl parameter.
  SILFunctionArgument *createFunctionArgument(SILType Ty,
                                              ValueDecl *D = nullptr,
                                              bool disableEntryBlockVerification = false);

  SILFunctionArgument *insertFunctionArgument(unsigned AtArgPos, SILType Ty,
                                              ValueOwnershipKind OwnershipKind,
                                              ValueDecl *D = nullptr);

  /// Replace the \p{i}th Function arg with a new Function arg with SILType \p
  /// Ty and ValueDecl \p D.
  SILFunctionArgument *replaceFunctionArgument(unsigned i, SILType Ty,
                                               ValueOwnershipKind Kind,
                                               ValueDecl *D = nullptr);

  /// Replace the \p{i}th BB arg with a new BBArg with SILType \p Ty and
  /// ValueDecl \p D.
  ///
  /// NOTE: This assumes that the current argument in position \p i has had its
  /// uses eliminated. To replace/replace all uses with, use
  /// replacePhiArgumentAndRAUW.
  SILPhiArgument *replacePhiArgument(unsigned i, SILType type,
                                     ValueOwnershipKind kind,
                                     ValueDecl *decl = nullptr,
                                     bool isReborrow = false,
                                     bool isEscaping = false);

  /// Replace phi argument \p i and RAUW all uses.
  SILPhiArgument *replacePhiArgumentAndReplaceAllUses(
      unsigned i, SILType type, ValueOwnershipKind kind,
      ValueDecl *decl = nullptr, bool isReborrow = false,
      bool isEscaping = false);

  /// Allocate a new argument of type \p Ty and append it to the argument
  /// list. Optionally you can pass in a value decl parameter, reborrow flag and
  /// escaping flag.
  SILPhiArgument *createPhiArgument(SILType Ty, ValueOwnershipKind Kind,
                                    ValueDecl *D = nullptr,
                                    bool isReborrow = false,
                                    bool isEscaping = false);

  /// Insert a new SILPhiArgument with type \p Ty and \p Decl at position \p
  /// AtArgPos.
  SILPhiArgument *insertPhiArgument(unsigned AtArgPos, SILType Ty,
                                    ValueOwnershipKind Kind,
                                    ValueDecl *D = nullptr,
                                    bool isReborrow = false,
                                    bool isEscaping = false);

  /// Remove all block arguments.
  void dropAllArguments();

  //===--------------------------------------------------------------------===//
  // Successors
  //===--------------------------------------------------------------------===//

  using SuccessorListTy = TermInst::SuccessorListTy;
  using ConstSuccessorListTy = TermInst::ConstSuccessorListTy;
  
  /// The successors of a SILBasicBlock are defined either explicitly as
  /// a single successor as the branch targets of the terminator instruction.
  ConstSuccessorListTy getSuccessors() const {
    return getTerminator()->getSuccessors();
  }
  SuccessorListTy getSuccessors() {
    return getTerminator()->getSuccessors();
  }

  using const_succ_iterator = TermInst::const_succ_iterator;
  using succ_iterator = TermInst::succ_iterator;

  bool succ_empty() const { return getTerminator()->succ_empty(); }
  succ_iterator succ_begin() { return getTerminator()->succ_begin(); }
  succ_iterator succ_end() { return getTerminator()->succ_end(); }
  const_succ_iterator succ_begin() const {
    return getTerminator()->succ_begin();
  }
  const_succ_iterator succ_end() const { return getTerminator()->succ_end(); }

  using succblock_iterator = TermInst::succblock_iterator;
  using const_succblock_iterator = TermInst::const_succblock_iterator;

  succblock_iterator succblock_begin() {
    return getTerminator()->succblock_begin();
  }
  succblock_iterator succblock_end() {
    return getTerminator()->succblock_end();
  }
  const_succblock_iterator succblock_begin() const {
    return getTerminator()->succblock_begin();
  }
  const_succblock_iterator succblock_end() const {
    return getTerminator()->succblock_end();
  }
  
  unsigned getNumSuccessors() const {
    return getTerminator()->getNumSuccessors();
  }

  SILBasicBlock *getSingleSuccessorBlock() {
    return getTerminator()->getSingleSuccessorBlock();
  }

  const SILBasicBlock *getSingleSuccessorBlock() const {
    return getTerminator()->getSingleSuccessorBlock();
  }

  using SuccessorBlockListTy = TermInst::SuccessorBlockListTy;
  using ConstSuccessorBlockListTy = TermInst::ConstSuccessorBlockListTy;

  /// Return the range of SILBasicBlocks that are successors of this block.
  SuccessorBlockListTy getSuccessorBlocks() {
    return getTerminator()->getSuccessorBlocks();
  }

  /// Return the range of SILBasicBlocks that are successors of this block.
  ConstSuccessorBlockListTy getSuccessorBlocks() const {
    return getTerminator()->getSuccessorBlocks();
  }

  //===--------------------------------------------------------------------===//
  // Predecessors
  //===--------------------------------------------------------------------===//

  using pred_iterator = SILSuccessor::pred_iterator;

  bool pred_empty() const { return PredList == nullptr; }
  pred_iterator pred_begin() const { return pred_iterator(PredList); }
  pred_iterator pred_end() const { return pred_iterator(); }

  iterator_range<pred_iterator> getPredecessorBlocks() const {
    return {pred_begin(), pred_end()};
  }

  SILBasicBlock *getSinglePredecessorBlock() {
    if (pred_empty() || std::next(pred_begin()) != pred_end())
      return nullptr;
    return *pred_begin();
  }

  const SILBasicBlock *getSinglePredecessorBlock() const {
    return const_cast<SILBasicBlock *>(this)->getSinglePredecessorBlock();
  }

  //===--------------------------------------------------------------------===//
  // Utility
  //===--------------------------------------------------------------------===//

  /// Returns true if this BB is the entry BB of its parent.
  bool isEntry() const;

  /// Returns true if this block ends in an unreachable or an apply of a
  /// no-return apply or builtin.
  bool isNoReturn() const;

  /// Returns true if this block only contains a branch instruction.
  bool isTrampoline() const;

  /// Returns true if it is legal to hoist instructions into this block.
  ///
  /// Used by toolchain::LoopInfo.
  bool isLegalToHoistInto() const;

  /// Returns the debug scope of the first non-meta instructions in the
  /// basic block. SILBuilderWithScope uses this to correctly set up
  /// the debug scope for newly created instructions.
  const SILDebugScope *getScopeOfFirstNonMetaInstruction();

  /// Whether the block has any phi arguments.
  ///
  /// Note that a block could have an argument and still return false.  The
  /// argument must also satisfy SILPhiArgument::isPhi.
  bool hasPhi() const;

  //===--------------------------------------------------------------------===//
  // Debugging
  //===--------------------------------------------------------------------===//

  /// Pretty-print the SILBasicBlock.
  void dump() const;

  /// Pretty-print the SILBasicBlock with Debug Info.
  void dump(bool DebugInfo) const;

  /// Pretty-print the SILBasicBlock with the designated stream.
  void print(toolchain::raw_ostream &OS) const;

  /// Pretty-print the SILBasicBlock with the designated context.
  void print(SILPrintContext &Ctx) const;

  void printAsOperand(raw_ostream &OS, bool PrintType = true);

  /// Print the ID of the block, bbN.
  void dumpID(bool newline = true) const;

  /// Print the ID of the block with \p OS, bbN.
  void printID(toolchain::raw_ostream &OS, bool newline = true) const;

  /// Print the ID of the block with \p Ctx, bbN.
  void printID(SILPrintContext &Ctx, bool newline = true) const;

  /// getSublistAccess() - returns pointer to member of instruction list
  static InstListType SILBasicBlock::*getSublistAccess() {
    return &SILBasicBlock::InstList;
  }

  /// Drops all uses that belong to this basic block.
  void dropAllReferences() {
    dropAllArguments();
    for (SILInstruction &I : *this)
      I.dropAllReferences();
  }

private:
  friend class SILArgument;

  /// BBArgument's ctor adds it to the argument list of this block.
  void insertArgument(arg_iterator Iter, SILArgument *Arg) {
    ArgumentList.insert(Iter, Arg);
  }
};

inline toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS,
                                     const SILBasicBlock &BB) {
  BB.print(OS);
  return OS;
}
} // end language namespace

namespace toolchain {

//===----------------------------------------------------------------------===//
// ilist_traits for SILBasicBlock
//===----------------------------------------------------------------------===//

template <>
struct ilist_traits<::language::SILBasicBlock>
  : ilist_node_traits<::language::SILBasicBlock> {
  using SelfTy = ilist_traits<::language::SILBasicBlock>;
  using SILBasicBlock = ::language::SILBasicBlock;
  using SILFunction = ::language::SILFunction;
  using FunctionPtrTy = ::language::NullablePtr<SILFunction>;

private:
  friend class ::language::SILFunction;

  SILFunction *Parent;
  using block_iterator = simple_ilist<SILBasicBlock>::iterator;

public:
  static void deleteNode(SILBasicBlock *BB) { BB->~SILBasicBlock(); }

  void transferNodesFromList(ilist_traits<SILBasicBlock> &SrcTraits,
                             block_iterator First, block_iterator Last);
private:
  static void createNode(const SILBasicBlock &);
};

} // end toolchain namespace

//===----------------------------------------------------------------------===//
//                           PhiOperand & PhiValue
//===----------------------------------------------------------------------===//

namespace language {

/// Represent a phi argument without storing pointers to branches or their
/// operands which are invalidated by adding new, unrelated phi values. Because
/// this only stores a block pointer, it remains valid as long as the CFG is
/// immutable and the index of the phi value does not change.
///
/// Note: this should not be confused with SILPhiArgument which should be
/// renamed to SILPhiValue and only used for actual phis.
///
/// Warning: This is invalid for CondBranchInst arguments. Clients assume that
/// any instructions inserted at the phi argument is post-dominated by that phi
/// argument. This warning can be removed once the SILVerifier fully prohibits
/// CondBranchInst arguments at all SIL stages.
struct PhiOperand {
  SILBasicBlock *predBlock = nullptr;
  unsigned argIndex = 0;

  PhiOperand() = default;

  // This if \p operand is a CondBrInst operand, then this constructs and
  // invalid PhiOperand. The abstraction only works for non-trivial OSSA values.
  PhiOperand(Operand *operand) {
    auto *branch = dyn_cast<BranchInst>(operand->getUser());
    if (!branch)
      return;

    predBlock = branch->getParent();
    argIndex = operand->getOperandNumber();
  }

  explicit operator bool() const { return predBlock != nullptr; }

  bool operator==(PhiOperand other) const {
    return predBlock == other.predBlock && argIndex == other.argIndex;
  }

  bool operator!=(PhiOperand other) const { return !(*this == other); }

  BranchInst *getBranch() const {
    return cast<BranchInst>(predBlock->getTerminator());
  }

  Operand *getOperand() const {
    return &getBranch()->getAllOperands()[argIndex];
  }

  SILPhiArgument *getValue() const {
    return
      cast<SILPhiArgument>(getBranch()->getDestBB()->getArgument(argIndex));
  }

  SILValue getSource() const {
    return getOperand()->get();
  }

  operator Operand *() const { return getOperand(); }
  Operand *operator*() const { return getOperand(); }
  Operand *operator->() const { return getOperand(); }
};

/// Represent a phi value without referencing the SILValue, which is invalidated
/// by adding new, unrelated phi values. Because this only stores a block
/// pointer, it remains valid as long as the CFG is immutable and the index of
/// the phi value does not change.
struct PhiValue {
  SILBasicBlock *phiBlock = nullptr;
  unsigned argIndex = 0;

  PhiValue() = default;

  PhiValue(SILValue value) {
    if (auto *blockArg = SILArgument::asPhi(value)) {
      phiBlock = blockArg->getParent();
      argIndex = blockArg->getIndex();
    }
  }

  explicit operator bool() const { return phiBlock != nullptr; }

  bool operator==(PhiValue other) const {
    return phiBlock == other.phiBlock && argIndex == other.argIndex;
  }

  bool operator!=(PhiValue other) const { return !(*this == other); }

  SILPhiArgument *getValue() const {
    return cast<SILPhiArgument>(phiBlock->getArgument(argIndex));
  }

  Operand *getOperand(SILBasicBlock *predecessor) {
    auto *term = predecessor->getTerminator();
    if (auto *branch = dyn_cast<BranchInst>(term)) {
      return &branch->getAllOperands()[argIndex];
    }
    // TODO: Support CondBr for legacy reasons
    return cast<CondBranchInst>(term)->getOperandForDestBB(phiBlock, argIndex);
  }

  operator SILValue() const { return getValue(); }
  SILValue operator*() const { return getValue(); }
  SILValue operator->() const { return getValue(); }
};

} // namespace language

namespace toolchain {

template <> struct DenseMapInfo<language::PhiOperand> {
  static language::PhiOperand getEmptyKey() { return language::PhiOperand(); }
  static language::PhiOperand getTombstoneKey() {
    language::PhiOperand phiOper;
    phiOper.predBlock =
        toolchain::DenseMapInfo<language::SILBasicBlock *>::getTombstoneKey();
    return phiOper;
  }
  static unsigned getHashValue(language::PhiOperand phiOper) {
    return toolchain::hash_combine(phiOper.predBlock, phiOper.argIndex);
  }
  static bool isEqual(language::PhiOperand lhs, language::PhiOperand rhs) {
    return lhs == rhs;
  }
};

template <> struct DenseMapInfo<language::PhiValue> {
  static language::PhiValue getEmptyKey() { return language::PhiValue(); }
  static language::PhiValue getTombstoneKey() {
    language::PhiValue phiValue;
    phiValue.phiBlock =
        toolchain::DenseMapInfo<language::SILBasicBlock *>::getTombstoneKey();
    return phiValue;
  }
  static unsigned getHashValue(language::PhiValue phiValue) {
    return toolchain::hash_combine(phiValue.phiBlock, phiValue.argIndex);
  }
  static bool isEqual(language::PhiValue lhs, language::PhiValue rhs) {
    return lhs == rhs;
  }
};

} // end namespace toolchain

//===----------------------------------------------------------------------===//
// Inline SILInstruction implementations
//===----------------------------------------------------------------------===//

namespace language {

inline SILFunction *SILInstruction::getFunction() const {
  return getParent()->getParent();
}

inline bool SILInstruction::visitPriorInstructions(
    toolchain::function_ref<bool(SILInstruction *)> visitor) {
  if (auto *previous = getPreviousInstruction()) {
    return visitor(previous);
  }
  for (auto *predecessor : getParent()->getPredecessorBlocks()) {
    if (!visitor(&predecessor->back()))
      return false;
  }
  return true;
}
inline bool SILInstruction::visitSubsequentInstructions(
    toolchain::function_ref<bool(SILInstruction *)> visitor) {
  if (auto *next = getNextInstruction()) {
    return visitor(next);
  }
  for (auto *successor : getParent()->getSuccessorBlocks()) {
    if (!visitor(&successor->front()))
      return false;
  }
  return true;
}

inline SILInstruction *SILInstruction::getPreviousInstruction() {
  auto pos = getIterator();
  return pos == getParent()->begin() ? nullptr : &*std::prev(pos);
}

inline SILInstruction *SILInstruction::getNextInstruction() {
  auto nextPos = std::next(getIterator());
  return nextPos == getParent()->end() ? nullptr : &*nextPos;
}

} // end language namespace

#endif
