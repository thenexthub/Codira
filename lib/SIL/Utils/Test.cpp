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
// This file defines test::FunctionTest.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/SIL/Test.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/raw_ostream.h"

using namespace toolchain;
using namespace language;
using namespace language::test;

namespace {

class Registry {
  StringMap<FunctionTest> registeredTests;
  CodiraNativeFunctionTestThunk thunk;

public:
  static Registry &get() {
    static Registry registry;
    return registry;
  }

  void registerFunctionTest(FunctionTest test, StringRef name) {
    auto inserted = registeredTests.insert({name, test}).second;
    assert(inserted);
    (void)inserted;
  }

  void registerFunctionTestThunk(CodiraNativeFunctionTestThunk thunk) {
    this->thunk = thunk;
  }

  CodiraNativeFunctionTestThunk getFunctionTestThunk() { return thunk; }

  FunctionTest getFunctionTest(StringRef name) {
    auto iter = registeredTests.find(name);
    if (iter == registeredTests.end()) {
      toolchain::errs() << "Found no test named " << name << "!\n";
      print(toolchain::errs());
    }
    return iter->getValue();
  }

  void print(raw_ostream &OS) const {
    OS << "test::Registry(" << this << ") with " << registeredTests.size()
       << " entries: {{\n";
    for (auto &stringMapEntry : registeredTests) {
      OS << "\t" << stringMapEntry.getKey() << " -> "
         << &stringMapEntry.getValue() << "\n";
    }
    OS << "}} test::Registry(" << this << ")\n";
  }

  void dump() const { print(toolchain::dbgs()); }
};

} // end anonymous namespace

void registerFunctionTestThunk(CodiraNativeFunctionTestThunk thunk) {
  Registry::get().registerFunctionTestThunk(thunk);
}

FunctionTest::FunctionTest(StringRef name, Invocation invocation)
    : invocation(invocation), pass(nullptr), function(nullptr),
      dependencies(nullptr) {
  Registry::get().registerFunctionTest(*this, name);
}
FunctionTest::FunctionTest(StringRef name, NativeCodiraInvocation invocation)
    : invocation(invocation), pass(nullptr), function(nullptr),
      dependencies(nullptr) {}

void FunctionTest::createNativeCodiraFunctionTest(
    StringRef name, NativeCodiraInvocation invocation) {
  Registry::get().registerFunctionTest({name, invocation}, name);
}

FunctionTest FunctionTest::get(StringRef name) {
  return Registry::get().getFunctionTest(name);
}

void FunctionTest::run(SILFunction &function, Arguments &arguments,
                       SILFunctionTransform &pass, Dependencies &dependencies) {
  this->pass = &pass;
  this->function = &function;
  this->dependencies = &dependencies;
  if (invocation.isa<Invocation>()) {
    auto fn = invocation.get<Invocation>();
    fn(function, arguments, *this);
  } else {
    toolchain::outs().flush();
    auto *fn = invocation.get<NativeCodiraInvocation>();
    Registry::get().getFunctionTestThunk()(fn, {&function}, {&arguments},
                                           {getCodiraPassInvocation()});
    fflush(stdout);
  }
  this->pass = nullptr;
  this->function = nullptr;
  this->dependencies = nullptr;
}

DominanceInfo *FunctionTest::getDominanceInfo() {
  return dependencies->getDominanceInfo();
}

DeadEndBlocks *FunctionTest::getDeadEndBlocks() {
  return dependencies->getDeadEndBlocks();
}

SILPassManager *FunctionTest::getPassManager() {
  return dependencies->getPassManager();
}

CodiraPassInvocation *FunctionTest::getCodiraPassInvocation() {
  return dependencies->getCodiraPassInvocation();
}
