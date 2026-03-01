//===- DialectLLVM.cpp - Pybind module for LLVM dialect API support -------===//
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

#include <string>

#include "mlir-c/Dialect/LLVM.h"
#include "mlir-c/IR.h"
#include "mlir-c/Support.h"
#include "mlir-c/Target/LLVMIR.h"
#include "mlir/Bindings/Python/Diagnostics.h"
#include "mlir/Bindings/Python/IRCore.h"
#include "mlir/Bindings/Python/Nanobind.h"
#include "mlir/Bindings/Python/NanobindAdaptors.h"

namespace nb = nanobind;

using namespace nanobind::literals;
using namespace vm::core;
using namespace mlir;
using namespace mlir::python::nanobind_adaptors;

namespace mlir {
namespace python {
namespace MLIR_BINDINGS_PYTHON_DOMAIN {
namespace vm::core {
//===--------------------------------------------------------------------===//
// StructType
//===--------------------------------------------------------------------===//

struct StructType : PyConcreteType<StructType> {
  static constexpr IsAFunctionTy isaFunction = mlirTypeIsALLVMStructType;
  static constexpr GetTypeIDFunctionTy getTypeIdFunction =
      mlirLLVMStructTypeGetTypeID;
  static constexpr const char *pyClassName = "StructType";
  static inline const MlirStringRef name = mlirLLVMStructTypeGetName();
  using Base::Base;

  static void bindDerived(ClassTy &c) {
    c.def_static(
        "get_literal",
        [](const std::vector<PyType> &elements, bool packed,
           DefaultingPyLocation loc, DefaultingPyMlirContext context) {
          python::CollectDiagnosticsToStringScope scope(
              mlirLocationGetContext(loc));
          std::vector<MlirType> elements_(elements.size());
          std::copy(elements.begin(), elements.end(), elements_.begin());

          MlirType type = mlirLLVMStructTypeLiteralGetChecked(
              loc, elements.size(), elements_.data(), packed);
          if (mlirTypeIsNull(type)) {
            throw nb::value_error(scope.takeMessage().c_str());
          }
          return StructType(context->getRef(), type);
        },
        "elements"_a, nb::kw_only(), "packed"_a = false, "loc"_a = nb::none(),
        "context"_a = nb::none());

    c.def_static(
        "get_literal_unchecked",
        [](const std::vector<PyType> &elements, bool packed,
           DefaultingPyMlirContext context) {
          python::CollectDiagnosticsToStringScope scope(context.get()->get());

          std::vector<MlirType> elements_(elements.size());
          std::copy(elements.begin(), elements.end(), elements_.begin());

          MlirType type = mlirLLVMStructTypeLiteralGet(
              context.get()->get(), elements.size(), elements_.data(), packed);
          if (mlirTypeIsNull(type)) {
            throw nb::value_error(scope.takeMessage().c_str());
          }
          return StructType(context->getRef(), type);
        },
        "elements"_a, nb::kw_only(), "packed"_a = false,
        "context"_a = nb::none());

    c.def_static(
        "get_identified",
        [](const std::string &name, DefaultingPyMlirContext context) {
          return StructType(context->getRef(),
                            mlirLLVMStructTypeIdentifiedGet(
                                context.get()->get(),
                                mlirStringRefCreate(name.data(), name.size())));
        },
        "name"_a, nb::kw_only(), "context"_a = nb::none());

    c.def_static(
        "get_opaque",
        [](const std::string &name, DefaultingPyMlirContext context) {
          return StructType(context->getRef(),
                            mlirLLVMStructTypeOpaqueGet(
                                context.get()->get(),
                                mlirStringRefCreate(name.data(), name.size())));
        },
        "name"_a, "context"_a = nb::none());

    c.def(
        "set_body",
        [](const StructType &self, const std::vector<PyType> &elements,
           bool packed) {
          std::vector<MlirType> elements_(elements.size());
          std::copy(elements.begin(), elements.end(), elements_.begin());
          MlirLogicalResult result = mlirLLVMStructTypeSetBody(
              self, elements.size(), elements_.data(), packed);
          if (!mlirLogicalResultIsSuccess(result)) {
            throw nb::value_error(
                "Struct body already set to different content.");
          }
        },
        "elements"_a, nb::kw_only(), "packed"_a = false);

    c.def_static(
        "new_identified",
        [](const std::string &name, const std::vector<PyType> &elements,
           bool packed, DefaultingPyMlirContext context) {
          std::vector<MlirType> elements_(elements.size());
          std::copy(elements.begin(), elements.end(), elements_.begin());
          return StructType(context->getRef(),
                            mlirLLVMStructTypeIdentifiedNewGet(
                                context.get()->get(),
                                mlirStringRefCreate(name.data(), name.length()),
                                elements.size(), elements_.data(), packed));
        },
        "name"_a, "elements"_a, nb::kw_only(), "packed"_a = false,
        "context"_a = nb::none());

    c.def_prop_ro(
        "name", [](const StructType &type) -> std::optional<std::string> {
          if (mlirLLVMStructTypeIsLiteral(type))
            return std::nullopt;

          MlirStringRef stringRef = mlirLLVMStructTypeGetIdentifier(type);
          return StringRef(stringRef.data, stringRef.length).str();
        });

    c.def_prop_ro("body", [](const StructType &type) -> nb::object {
      // Don't crash in absence of a body.
      if (mlirLLVMStructTypeIsOpaque(type))
        return nb::none();

      nb::list body;
      for (intptr_t i = 0, e = mlirLLVMStructTypeGetNumElementTypes(type);
           i < e; ++i) {
        body.append(mlirLLVMStructTypeGetElementType(type, i));
      }
      return body;
    });

    c.def_prop_ro("packed", [](const StructType &type) {
      return mlirLLVMStructTypeIsPacked(type);
    });

    c.def_prop_ro("opaque", [](const StructType &type) {
      return mlirLLVMStructTypeIsOpaque(type);
    });
  }
};

//===--------------------------------------------------------------------===//
// PointerType
//===--------------------------------------------------------------------===//

struct PointerType : PyConcreteType<PointerType> {
  static constexpr IsAFunctionTy isaFunction = mlirTypeIsALLVMPointerType;
  static constexpr GetTypeIDFunctionTy getTypeIdFunction =
      mlirLLVMPointerTypeGetTypeID;
  static constexpr const char *pyClassName = "PointerType";
  static inline const MlirStringRef name = mlirLLVMPointerTypeGetName();
  using Base::Base;

  static void bindDerived(ClassTy &c) {
    c.def_static(
        "get",
        [](std::optional<unsigned> addressSpace,
           DefaultingPyMlirContext context) {
          python::CollectDiagnosticsToStringScope scope(context.get()->get());
          MlirType type = mlirLLVMPointerTypeGet(
              context.get()->get(),
              addressSpace.has_value() ? *addressSpace : 0);
          if (mlirTypeIsNull(type)) {
            throw nb::value_error(scope.takeMessage().c_str());
          }
          return PointerType(context->getRef(), type);
        },
        "address_space"_a = nb::none(), nb::kw_only(),
        "context"_a = nb::none());
    c.def_prop_ro("address_space", [](const PointerType &type) {
      return mlirLLVMPointerTypeGetAddressSpace(type);
    });
  }
};

static void populateDialectLLVMSubmodule(nanobind::module_ &m) {
  StructType::bind(m);
  PointerType::bind(m);

  m.def(
      "translate_module_to_llvmir",
      [](const PyOperation &module) {
        return mlirTranslateModuleToLLVMIRToString(module);
      },
      "module"_a, nb::rv_policy::take_ownership);
}
} // namespace vm::core
} // namespace MLIR_BINDINGS_PYTHON_DOMAIN
} // namespace python
} // namespace mlir

NB_MODULE(_mlirDialectsLLVM, m) {
  m.doc() = "MLIR LLVM Dialect";

  mlir::python::MLIR_BINDINGS_PYTHON_DOMAIN::toolchain::populateDialectLLVMSubmodule(
      m);
}
