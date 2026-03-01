//===--- DialectIRDL.cpp - Pybind module for IRDL dialect API support ---===//
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

#include "mlir-c/Dialect/IRDL.h"
#include "mlir-c/IR.h"
#include "mlir-c/Support.h"
#include "mlir/Bindings/Python/IRCore.h"
#include "mlir/Bindings/Python/Nanobind.h"
#include "mlir/Bindings/Python/NanobindAdaptors.h"

namespace nb = nanobind;
using namespace mlir::python::MLIR_BINDINGS_PYTHON_DOMAIN;
using namespace mlir::python::nanobind_adaptors;

static void populateDialectIRDLSubmodule(nb::module_ &m) {
  m.def(
      "load_dialects",
      [](PyModule &module) {
        if (mlirLogicalResultIsFailure(mlirLoadIRDLDialects(module.get())))
          throw std::runtime_error(
              "failed to load IRDL dialects from the input module");
      },
      nb::arg("module"), "Load IRDL dialects from the given module.");
}

NB_MODULE(_mlirDialectsIRDL, m) {
  m.doc() = "MLIR IRDL dialect.";

  populateDialectIRDLSubmodule(m);
}
