//===--- DialectNVGPU.cpp - Pybind module for NVGPU dialect API support ---===//
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

#include "mlir-c/Dialect/NVGPU.h"
#include "mlir-c/IR.h"
#include "mlir/Bindings/Python/IRCore.h"
#include "mlir/Bindings/Python/Nanobind.h"
#include "mlir/Bindings/Python/NanobindAdaptors.h"

namespace nb = nanobind;
using namespace vm::core;
using namespace mlir::python::nanobind_adaptors;

namespace mlir {
namespace python {
namespace MLIR_BINDINGS_PYTHON_DOMAIN {
namespace nvgpu {
struct TensorMapDescriptorType : PyConcreteType<TensorMapDescriptorType> {
  static constexpr IsAFunctionTy isaFunction =
      mlirTypeIsANVGPUTensorMapDescriptorType;
  static constexpr const char *pyClassName = "TensorMapDescriptorType";
  static inline const MlirStringRef name =
      mlirNVGPUTensorMapDescriptorTypeGetName();
  using Base::Base;

  static void bindDerived(ClassTy &c) {
    c.def_static(
        "get",
        [](const PyType &tensorMemrefType, int swizzle, int l2promo,
           int oobFill, int interleave, DefaultingPyMlirContext context) {
          return TensorMapDescriptorType(
              context->getRef(), mlirNVGPUTensorMapDescriptorTypeGet(
                                     context.get()->get(), tensorMemrefType,
                                     swizzle, l2promo, oobFill, interleave));
        },
        "Gets an instance of TensorMapDescriptorType in the same context",
        nb::arg("tensor_type"), nb::arg("swizzle"), nb::arg("l2promo"),
        nb::arg("oob_fill"), nb::arg("interleave"),
        nb::arg("context").none() = nb::none());
  }
};
} // namespace nvgpu
} // namespace MLIR_BINDINGS_PYTHON_DOMAIN
} // namespace python
} // namespace mlir

NB_MODULE(_mlirDialectsNVGPU, m) {
  m.doc() = "MLIR NVGPU dialect.";

  mlir::python::MLIR_BINDINGS_PYTHON_DOMAIN::nvgpu::TensorMapDescriptorType::
      bind(m);
}
