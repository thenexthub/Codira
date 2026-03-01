//===- MainModule.cpp - Main pybind module --------------------------------===//
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

#include "Pass.h"
#include "Rewrite.h"
#include "mlir/Bindings/Python/Globals.h"
#include "mlir/Bindings/Python/IRAttributes.h"
#include "mlir/Bindings/Python/IRCore.h"
#include "mlir/Bindings/Python/IRTypes.h"
#include "mlir/Bindings/Python/Nanobind.h"

namespace nb = nanobind;
using namespace mlir::python::MLIR_BINDINGS_PYTHON_DOMAIN;

namespace mlir {
namespace python {
namespace MLIR_BINDINGS_PYTHON_DOMAIN {
void populateIRAffine(nb::module_ &m);
void populateIRAttributes(nb::module_ &m);
void populateIRInterfaces(nb::module_ &m);
void populateIRTypes(nb::module_ &m);
void populateIRCore(nb::module_ &m);
void populateRoot(nb::module_ &m);
} // namespace MLIR_BINDINGS_PYTHON_DOMAIN
} // namespace python
} // namespace mlir

// -----------------------------------------------------------------------------
// Module initialization.
// -----------------------------------------------------------------------------
NB_MODULE(_mlir, m) {
  // disable leak warnings which tend to be false positives.
  nb::set_leak_warnings(false);

  m.doc() = "MLIR Python Native Extension";
  populateRoot(m);
  // Define and populate IR submodule.
  auto irModule = m.def_submodule("ir", "MLIR IR Bindings");
  populateIRCore(irModule);
  populateIRAffine(irModule);
  populateIRAttributes(irModule);
  populateIRInterfaces(irModule);
  populateIRTypes(irModule);

  auto rewriteModule = m.def_submodule("rewrite", "MLIR Rewrite Bindings");
  populateRewriteSubmodule(rewriteModule);

  // Define and populate PassManager submodule.
  auto passManagerModule =
      m.def_submodule("passmanager", "MLIR Pass Management Bindings");
  populatePassManagerSubmodule(passManagerModule);
  nanobind::register_exception_translator(
      [](const std::exception_ptr &p, void *payload) {
        // We can't define exceptions with custom fields through pybind, so
        // instead the exception class is defined in python and imported here.
        try {
          if (p)
            std::rethrow_exception(p);
        } catch (const MLIRError &e) {
          nanobind::object obj =
              nanobind::module_::import_(MAKE_MLIR_PYTHON_QUALNAME("ir"))
                  .attr("MLIRError")(e.message, e.errorDiagnostics);
          PyErr_SetObject(PyExc_Exception, obj.ptr());
        }
      });
}
