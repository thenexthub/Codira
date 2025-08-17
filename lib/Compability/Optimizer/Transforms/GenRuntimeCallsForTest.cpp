//===- GenRuntimeCallsForTest.cpp -----------------------------------------===//
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

//===----------------------------------------------------------------------===//
/// \file
/// This pass is only for developers to generate declarations/calls
/// of Fortran runtime function recognized in
/// flang/Optimizer/Transforms/RuntimeFunctions.inc table.
/// Sample of the generated FIR:
///   func.func private
///       @_FortranAioSetStatus(!fir.ref<i8>, !fir.ref<i8>, i64) ->
///       i1 attributes {fir.io, fir.runtime}
///
///   func.func @test__FortranAioSetStatus(
///       %arg0: !fir.ref<i8>, %arg1: !fir.ref<i8>, %arg2: i64) -> i1 {
///    %0 = fir.call @_FortranAioSetStatus(%arg0, %arg1, %arg2) :
///        (!fir.ref<i8>, !fir.ref<i8>, i64) -> i1
///    return %0 : i1
///  }
//===----------------------------------------------------------------------===//
#include "language/Compability/Common/static-multimap-view.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "language/Compability/Runtime/io-api.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"

namespace fir {
#define GEN_PASS_DEF_GENRUNTIMECALLSFORTEST
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

#define DEBUG_TYPE "gen-runtime-calls-for-test"

using namespace language::Compability::runtime;
using namespace language::Compability::runtime::io;

#define mkIOKey(X) FirmkKey(IONAME(X))
#define mkRTKey(X) FirmkKey(RTNAME(X))

namespace {
class GenRuntimeCallsForTestPass
    : public fir::impl::GenRuntimeCallsForTestBase<GenRuntimeCallsForTestPass> {
  using GenRuntimeCallsForTestBase<
      GenRuntimeCallsForTestPass>::GenRuntimeCallsForTestBase;

public:
  void runOnOperation() override;
};
} // end anonymous namespace

static constexpr toolchain::StringRef testPrefix = "test_";

void GenRuntimeCallsForTestPass::runOnOperation() {
  mlir::ModuleOp moduleOp = getOperation();
  mlir::OpBuilder mlirBuilder(moduleOp.getRegion());
  fir::FirOpBuilder builder(mlirBuilder, moduleOp);
  mlir::Location loc = mlir::UnknownLoc::get(builder.getContext());

#define KNOWN_IO_FUNC(X)                                                       \
  fir::runtime::getIORuntimeFunc<mkIOKey(X)>(loc, builder)
#define KNOWN_RUNTIME_FUNC(X)                                                  \
  fir::runtime::getRuntimeFunc<mkRTKey(X)>(loc, builder)

  mlir::func::FuncOp runtimeFuncsTable[] = {
#include "language/Compability/Optimizer/Transforms/RuntimeFunctions.inc"
  };

  if (!doGenerateCalls)
    return;

  // Generate thin wrapper functions calling the known Fortran
  // runtime functions.
  toolchain::SmallVector<mlir::Operation *> newFuncs;
  for (unsigned i = 0;
       i < sizeof(runtimeFuncsTable) / sizeof(runtimeFuncsTable[0]); ++i) {
    mlir::func::FuncOp funcOp = runtimeFuncsTable[i];
    mlir::FunctionType funcTy = funcOp.getFunctionType();
    std::string name = (toolchain::Twine(testPrefix) + funcOp.getName()).str();
    mlir::func::FuncOp callerFunc = builder.createFunction(loc, name, funcTy);
    callerFunc.setVisibility(mlir::SymbolTable::Visibility::Public);
    mlir::OpBuilder::InsertPoint insertPt = builder.saveInsertionPoint();

    // Generate the wrapper function body that consists of a call and return.
    builder.setInsertionPointToStart(callerFunc.addEntryBlock());
    mlir::Block::BlockArgListType args = callerFunc.front().getArguments();
    auto callOp = fir::CallOp::create(builder, loc, funcOp, args);
    mlir::func::ReturnOp::create(builder, loc, callOp.getResults());

    newFuncs.push_back(callerFunc.getOperation());
    builder.restoreInsertionPoint(insertPt);
  }

  // Make sure all wrapper functions are at the beginning
  // of the module.
  auto moduleBegin = moduleOp.getBody()->begin();
  for (auto func : newFuncs)
    func->moveBefore(moduleOp.getBody(), moduleBegin);
}
