//===- TransformInterpreter.cpp -------------------------------------------===//
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
//
// Pybind classes for the transform dialect interpreter.
//
//===----------------------------------------------------------------------===//

#include "mlir-c/Dialect/Transform/Interpreter.h"
#include "mlir-c/IR.h"
#include "mlir-c/Support.h"
#include "mlir/Bindings/Python/Diagnostics.h"
#include "mlir/Bindings/Python/IRCore.h"
#include "mlir/Bindings/Python/Nanobind.h"
#include "mlir/Bindings/Python/NanobindAdaptors.h"

namespace nb = nanobind;

namespace mlir {
namespace python {
namespace MLIR_BINDINGS_PYTHON_DOMAIN {
namespace transform_interpreter {
struct PyTransformOptions {
  PyTransformOptions() { options = mlirTransformOptionsCreate(); };
  PyTransformOptions(PyTransformOptions &&other) {
    options = other.options;
    other.options.ptr = nullptr;
  }
  PyTransformOptions(const PyTransformOptions &) = delete;

  ~PyTransformOptions() { mlirTransformOptionsDestroy(options); }

  MlirTransformOptions options;
};
} // namespace transform_interpreter
} // namespace MLIR_BINDINGS_PYTHON_DOMAIN
} // namespace python
} // namespace mlir

static void populateTransformInterpreterSubmodule(nb::module_ &m) {
  using namespace mlir::python::MLIR_BINDINGS_PYTHON_DOMAIN;
  using namespace transform_interpreter;
  nb::class_<PyTransformOptions>(m, "TransformOptions")
      .def(nb::init<>())
      .def_prop_rw(
          "expensive_checks",
          [](const PyTransformOptions &self) {
            return mlirTransformOptionsGetExpensiveChecksEnabled(self.options);
          },
          [](PyTransformOptions &self, bool value) {
            mlirTransformOptionsEnableExpensiveChecks(self.options, value);
          })
      .def_prop_rw(
          "enforce_single_top_level_transform_op",
          [](const PyTransformOptions &self) {
            return mlirTransformOptionsGetEnforceSingleTopLevelTransformOp(
                self.options);
          },
          [](PyTransformOptions &self, bool value) {
            mlirTransformOptionsEnforceSingleTopLevelTransformOp(self.options,
                                                                 value);
          });

  m.def(
      "apply_named_sequence",
      [](PyOperationBase &payloadRoot, PyOperationBase &transformRoot,
         PyOperationBase &transformModule, const PyTransformOptions &options) {
        mlir::python::CollectDiagnosticsToStringScope scope(
            mlirOperationGetContext(transformRoot.getOperation()));

        // Calling back into Python to invalidate everything under the payload
        // root. This is awkward, but we don't have access to PyMlirContext
        // object here otherwise.
        nb::object obj = nb::cast(payloadRoot);

        MlirLogicalResult result = mlirTransformApplyNamedSequence(
            payloadRoot.getOperation(), transformRoot.getOperation(),
            transformModule.getOperation(), options.options);
        if (mlirLogicalResultIsSuccess(result)) {
          // Even in cases of success, we might have diagnostics to report:
          std::string msg;
          if ((msg = scope.takeMessage()).size() > 0) {
            fprintf(stderr,
                    "Diagnostic generated while applying "
                    "transform.named_sequence:\n%s",
                    msg.data());
          }
          return;
        }

        throw nb::value_error(
            ("Failed to apply named transform sequence.\nDiagnostic message " +
             scope.takeMessage())
                .c_str());
      },
      nb::arg("payload_root"), nb::arg("transform_root"),
      nb::arg("transform_module"),
      nb::arg("transform_options") = PyTransformOptions());

  m.def(
      "copy_symbols_and_merge_into",
      [](PyOperationBase &target, PyOperationBase &other) {
        mlir::python::CollectDiagnosticsToStringScope scope(
            mlirOperationGetContext(target.getOperation()));

        MlirLogicalResult result = mlirMergeSymbolsIntoFromClone(
            target.getOperation(), other.getOperation());
        if (mlirLogicalResultIsFailure(result)) {
          throw nb::value_error(
              ("Failed to merge symbols.\nDiagnostic message " +
               scope.takeMessage())
                  .c_str());
        }
      },
      nb::arg("target"), nb::arg("other"));
}

NB_MODULE(_mlirTransformInterpreter, m) {
  m.doc() = "MLIR Transform dialect interpreter functionality.";
  populateTransformInterpreterSubmodule(m);
}
