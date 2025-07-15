//===- ParseTestSpecification.h - Parsing for test instructions -*- C++ -*-===//
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
// This file defines test::Argument.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_PARSETESTSPECIFICATION
#define LANGUAGE_SIL_PARSETESTSPECIFICATION

#include "language/Basic/TaggedUnion.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/FormattedStream.h"

namespace toolchain {
template <class T>
class SmallVectorImpl;
}
using toolchain::StringRef;

namespace language {

class SILFunction;
class SILArgument;

namespace test {

struct Argument {
  enum class Kind {
    String,
    Bool,
    UInt,
    Value,
    Operand,
    Instruction,
    BlockArgument,
    Block,
    Function,
  };
  using Union = TaggedUnion<StringRef,          // StringArgument
                            bool,               // BoolArgument
                            unsigned long long, // UIntArgument
                            SILValue,           // ValueArgument
                            Operand *,          // OperandArgument
                            SILInstruction *,   // InstructionArgument
                            SILArgument *,      // BlockArgumentArgument
                            SILBasicBlock *,    // BlockArgument
                            SILFunction *       // FunctionArgument
                            >;

private:
  Kind kind;

protected:
  Argument(Union storage, Kind kind) : kind(kind), storage(storage) {}
  Union storage;

public:
  Kind getKind() const { return kind; }
  void print(toolchain::raw_ostream &os);
#ifndef NDEBUG
  void dump() { print(toolchain::errs()); }
#endif
};

template <typename _Stored, Argument::Kind TheKind>
struct ConcreteArgument : Argument {
  using Stored = _Stored;
  using Super = ConcreteArgument<Stored, TheKind>;
  ConcreteArgument(Stored stored) : Argument(Union{stored}, TheKind) {}
  Stored getValue() { return storage.get<Stored>(); }
  static bool classof(const Argument *argument) {
    return argument->getKind() == TheKind;
  }
};

struct ValueArgument : ConcreteArgument<SILValue, Argument::Kind::Value> {
  ValueArgument(SILValue stored) : Super(stored) {}
};

struct OperandArgument : ConcreteArgument<Operand *, Argument::Kind::Operand> {
  OperandArgument(Operand *stored) : Super(stored) {}
};

struct InstructionArgument
    : ConcreteArgument<SILInstruction *, Argument::Kind::Instruction> {
  InstructionArgument(SILInstruction *stored) : Super(stored) {}
};

struct BlockArgumentArgument
    : ConcreteArgument<SILArgument *, Argument::Kind::BlockArgument> {
  BlockArgumentArgument(SILArgument *stored) : Super(stored) {}
};

struct BlockArgument
    : ConcreteArgument<SILBasicBlock *, Argument::Kind::Block> {
  BlockArgument(SILBasicBlock *stored) : Super(stored) {}
};

struct FunctionArgument
    : ConcreteArgument<SILFunction *, Argument::Kind::Function> {
  FunctionArgument(SILFunction *stored) : Super(stored) {}
};

struct BoolArgument : ConcreteArgument<bool, Argument::Kind::Bool> {
  BoolArgument(bool stored) : Super(stored) {}
};

struct UIntArgument
    : ConcreteArgument<unsigned long long, Argument::Kind::UInt> {
  UIntArgument(unsigned long long stored) : Super(stored) {}
};

struct StringArgument : ConcreteArgument<StringRef, Argument::Kind::String> {
  StringArgument(StringRef stored) : Super(stored) {}
};

struct Arguments {
  toolchain::SmallVector<Argument, 8> storage;
  unsigned untakenIndex = 0;

  void assertUsed() {
    assert(untakenIndex == storage.size() && "arguments remain!");
  }

  void clear() {
    assertUsed();
    storage.clear();
    untakenIndex = 0;
  }

  bool hasUntaken() { return untakenIndex < storage.size(); }

  Arguments() {}
  Arguments(Arguments const &) = delete;
  Arguments &operator=(Arguments const &) = delete;
  ~Arguments() { assertUsed(); }
  Argument &takeArgument() { return storage[untakenIndex++]; }

private:
  template <typename Subtype>
  typename Subtype::Stored getInstance(StringRef name, Argument &argument) {
    if (isa<Subtype>(argument)) {
      auto stored = cast<Subtype>(argument).getValue();
      return stored;
    }
    toolchain::errs() << "Attempting to take a " << name << " argument but have\n";
    argument.print(toolchain::errs());
    toolchain::report_fatal_error("Bad unit test");
  }
  template <typename Subtype>
  typename Subtype::Stored takeInstance(StringRef name) {
    auto argument = takeArgument();
    return getInstance<Subtype>(name, argument);
  }

public:
  StringRef takeString() { return takeInstance<StringArgument>("string"); }
  bool takeBool() { return takeInstance<BoolArgument>("bool"); }
  unsigned long long takeUInt() { return takeInstance<UIntArgument>("uint"); }
  SILValue takeValue() {
    auto argument = takeArgument();
    if (isa<InstructionArgument>(argument)) {
      auto *instruction = cast<InstructionArgument>(argument).getValue();
      auto *svi = cast<SingleValueInstruction>(instruction);
      return svi;
    } else if (isa<BlockArgumentArgument>(argument)) {
      auto *arg = cast<BlockArgumentArgument>(argument).getValue();
      return arg;
    }
    return getInstance<ValueArgument>("value", argument);
  }
  Operand *takeOperand() { return takeInstance<OperandArgument>("operand"); }
  SILInstruction *takeInstruction() {
    auto argument = takeArgument();
    if (isa<ValueArgument>(argument)) {
      auto value = cast<ValueArgument>(argument).getValue();
      auto *definingInstruction = value.getDefiningInstruction();
      assert(definingInstruction &&
             "selected instruction via argument value!?");
      return definingInstruction;
    }
    return getInstance<InstructionArgument>("instruction", argument);
  }
  SILArgument *takeBlockArgument() {
    return takeInstance<BlockArgumentArgument>("block argument");
  }
  SILBasicBlock *takeBlock() { return takeInstance<BlockArgument>("block"); }
  SILFunction *takeFunction() {
    return takeInstance<FunctionArgument>("block");
  }
};

/// The specification for a test which has not yet been parsed.
struct UnparsedSpecification {
  /// The string which specifies the test.
  ///
  /// Not a StringRef because the SpecifyTestInst whose payload is of
  /// interest gets deleted.
  std::string string;
  /// The next non-debug instruction.
  ///
  /// Provides an "anchor" for the specification.  Contextual arguments
  /// (@{instruction|block|function}) can be parsed in terms of this
  /// anchor.
  SILInstruction *context;
  /// Map from names used in the specification to the corresponding SILValues.
  toolchain::StringMap<SILValue> values;
};

/// Populates the array \p components with the elements of \p
/// specificationString.
void getTestSpecificationComponents(StringRef specificationString,
                                    SmallVectorImpl<StringRef> &components);

/// Finds and deletes each specify_test instruction in \p function and
/// appends its string payload to the provided vector.
void getTestSpecifications(
    SILFunction *function,
    SmallVectorImpl<UnparsedSpecification> &specifications);

/// Given the string \p specification operand of a specify_test
/// instruction from \p function, parse the arguments which it refers to into
/// \p arguments and the component strings into \p argumentStrings.
void parseTestArgumentsFromSpecification(
    SILFunction *function, UnparsedSpecification const &specification,
    Arguments &arguments, SmallVectorImpl<StringRef> &argumentStrings);

} // namespace test
} // namespace language

#endif
