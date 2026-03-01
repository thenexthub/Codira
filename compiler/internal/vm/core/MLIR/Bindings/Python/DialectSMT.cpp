//===- DialectSMT.cpp - Pybind module for SMT dialect API support ---------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "mlir/Bindings/Python/NanobindUtils.h"

#include "mlir-c/Dialect/SMT.h"
#include "mlir-c/IR.h"
#include "mlir-c/Support.h"
#include "mlir-c/Target/ExportSMTLIB.h"
#include "mlir/Bindings/Python/Diagnostics.h"
#include "mlir/Bindings/Python/IRCore.h"
#include "mlir/Bindings/Python/Nanobind.h"
#include "mlir/Bindings/Python/NanobindAdaptors.h"

namespace nb = nanobind;

using namespace nanobind::literals;
using namespace mlir;
using namespace mlir::python::nanobind_adaptors;

namespace mlir {
namespace python {
namespace MLIR_BINDINGS_PYTHON_DOMAIN {
namespace smt {
struct BoolType : PyConcreteType<BoolType> {
  static constexpr IsAFunctionTy isaFunction = mlirSMTTypeIsABool;
  static constexpr const char *pyClassName = "BoolType";
  static inline const MlirStringRef name = mlirSMTBoolTypeGetName();
  using Base::Base;

  static void bindDerived(ClassTy &c) {
    c.def_static(
        "get",
        [](DefaultingPyMlirContext context) {
          return BoolType(context->getRef(),
                          mlirSMTTypeGetBool(context.get()->get()));
        },
        nb::arg("context").none() = nb::none());
  }
};

struct BitVectorType : PyConcreteType<BitVectorType> {
  static constexpr IsAFunctionTy isaFunction = mlirSMTTypeIsABitVector;
  static constexpr const char *pyClassName = "BitVectorType";
  static inline const MlirStringRef name = mlirSMTBitVectorTypeGetName();
  using Base::Base;

  static void bindDerived(ClassTy &c) {
    c.def_static(
        "get",
        [](int32_t width, DefaultingPyMlirContext context) {
          return BitVectorType(
              context->getRef(),
              mlirSMTTypeGetBitVector(context.get()->get(), width));
        },
        nb::arg("width"), nb::arg("context").none() = nb::none());
  }
};

struct IntType : PyConcreteType<IntType> {
  static constexpr IsAFunctionTy isaFunction = mlirSMTTypeIsAInt;
  static constexpr const char *pyClassName = "IntType";
  static inline const MlirStringRef name = mlirSMTIntTypeGetName();
  using Base::Base;

  static void bindDerived(ClassTy &c) {
    c.def_static(
        "get",
        [](DefaultingPyMlirContext context) {
          return IntType(context->getRef(),
                         mlirSMTTypeGetInt(context.get()->get()));
        },
        nb::arg("context").none() = nb::none());
  }
};

static void populateDialectSMTSubmodule(nanobind::module_ &m) {
  BoolType::bind(m);
  BitVectorType::bind(m);
  IntType::bind(m);

  auto exportSMTLIB = [](MlirOperation module, bool inlineSingleUseValues,
                         bool indentLetBody) {
    CollectDiagnosticsToStringScope scope(mlirOperationGetContext(module));
    PyPrintAccumulator printAccum;
    MlirLogicalResult result = mlirTranslateOperationToSMTLIB(
        module, printAccum.getCallback(), printAccum.getUserData(),
        inlineSingleUseValues, indentLetBody);
    if (mlirLogicalResultIsSuccess(result))
      return printAccum.join();
    throw nb::value_error(
        ("Failed to export smtlib.\nDiagnostic message " + scope.takeMessage())
            .c_str());
  };

  m.def(
      "export_smtlib",
      [&exportSMTLIB](const PyOperation &module, bool inlineSingleUseValues,
                      bool indentLetBody) {
        return exportSMTLIB(module, inlineSingleUseValues, indentLetBody);
      },
      "module"_a, "inline_single_use_values"_a = false,
      "indent_let_body"_a = false);
  m.def(
      "export_smtlib",
      [&exportSMTLIB](PyModule &module, bool inlineSingleUseValues,
                      bool indentLetBody) {
        return exportSMTLIB(mlirModuleGetOperation(module.get()),
                            inlineSingleUseValues, indentLetBody);
      },
      "module"_a, "inline_single_use_values"_a = false,
      "indent_let_body"_a = false);
}
} // namespace smt
} // namespace MLIR_BINDINGS_PYTHON_DOMAIN
} // namespace python
} // namespace mlir

NB_MODULE(_mlirDialectsSMT, m) {
  m.doc() = "MLIR SMT Dialect";

  python::MLIR_BINDINGS_PYTHON_DOMAIN::smt::populateDialectSMTSubmodule(m);
}
