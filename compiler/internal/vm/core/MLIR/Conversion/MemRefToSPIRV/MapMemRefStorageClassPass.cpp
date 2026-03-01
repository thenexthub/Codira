//===- MapMemRefStorageCLassPass.cpp --------------------------------------===//
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
// This file implements a pass to map numeric MemRef memory spaces to
// symbolic ones defined in the SPIR-V specification.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/MemRefToSPIRV/MemRefToSPIRVPass.h"

#include "mlir/Conversion/MemRefToSPIRV/MemRefToSPIRV.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVAttributes.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVEnums.h"
#include "mlir/Dialect/SPIRV/IR/TargetAndABI.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Visitors.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "vm/core/ADT/SmallVectorExtras.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/Debug.h"
#include <optional>

namespace mlir {
#define GEN_PASS_DEF_MAPMEMREFSTORAGECLASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

#define DEBUG_TYPE "mlir-map-memref-storage-class"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Mappings
//===----------------------------------------------------------------------===//

/// Mapping between SPIR-V storage classes to memref memory spaces.
///
/// Note: memref does not have a defined semantics for each memory space; it
/// depends on the context where it is used. There are no particular reasons
/// behind the number assignments; we try to follow NVVM conventions and largely
/// give common storage classes a smaller number.
#define VULKAN_STORAGE_SPACE_MAP_LIST(MAP_FN)                                  \
  MAP_FN(spirv::StorageClass::StorageBuffer, 0)                                \
  MAP_FN(spirv::StorageClass::Generic, 1)                                      \
  MAP_FN(spirv::StorageClass::Workgroup, 3)                                    \
  MAP_FN(spirv::StorageClass::Uniform, 4)                                      \
  MAP_FN(spirv::StorageClass::Private, 5)                                      \
  MAP_FN(spirv::StorageClass::Function, 6)                                     \
  MAP_FN(spirv::StorageClass::PushConstant, 7)                                 \
  MAP_FN(spirv::StorageClass::UniformConstant, 8)                              \
  MAP_FN(spirv::StorageClass::Input, 9)                                        \
  MAP_FN(spirv::StorageClass::Output, 10)                                      \
  MAP_FN(spirv::StorageClass::PhysicalStorageBuffer, 11)                       \
  MAP_FN(spirv::StorageClass::Image, 12)

std::optional<spirv::StorageClass>
spirv::mapMemorySpaceToVulkanStorageClass(Attribute memorySpaceAttr) {
  // Handle null memory space attribute specially.
  if (!memorySpaceAttr)
    return spirv::StorageClass::StorageBuffer;

  // Unknown dialect custom attributes are not supported by default.
  // Downstream callers should plug in more specialized ones.
  auto intAttr = dyn_cast<IntegerAttr>(memorySpaceAttr);
  if (!intAttr)
    return std::nullopt;
  unsigned memorySpace = intAttr.getInt();

#define STORAGE_SPACE_MAP_FN(storage, space)                                   \
  case space:                                                                  \
    return storage;

  switch (memorySpace) {
    VULKAN_STORAGE_SPACE_MAP_LIST(STORAGE_SPACE_MAP_FN)
  default:
    break;
  }
  return std::nullopt;

#undef STORAGE_SPACE_MAP_FN
}

std::optional<unsigned>
spirv::mapVulkanStorageClassToMemorySpace(spirv::StorageClass storageClass) {
#define STORAGE_SPACE_MAP_FN(storage, space)                                   \
  case storage:                                                                \
    return space;

  switch (storageClass) {
    VULKAN_STORAGE_SPACE_MAP_LIST(STORAGE_SPACE_MAP_FN)
  default:
    break;
  }
  return std::nullopt;

#undef STORAGE_SPACE_MAP_FN
}

#undef VULKAN_STORAGE_SPACE_MAP_LIST

#define OPENCL_STORAGE_SPACE_MAP_LIST(MAP_FN)                                  \
  MAP_FN(spirv::StorageClass::CrossWorkgroup, 0)                               \
  MAP_FN(spirv::StorageClass::Generic, 1)                                      \
  MAP_FN(spirv::StorageClass::Workgroup, 3)                                    \
  MAP_FN(spirv::StorageClass::UniformConstant, 4)                              \
  MAP_FN(spirv::StorageClass::Private, 5)                                      \
  MAP_FN(spirv::StorageClass::Function, 6)                                     \
  MAP_FN(spirv::StorageClass::Image, 7)

std::optional<spirv::StorageClass>
spirv::mapMemorySpaceToOpenCLStorageClass(Attribute memorySpaceAttr) {
  // Handle null memory space attribute specially.
  if (!memorySpaceAttr)
    return spirv::StorageClass::CrossWorkgroup;

  // Unknown dialect custom attributes are not supported by default.
  // Downstream callers should plug in more specialized ones.
  auto intAttr = dyn_cast<IntegerAttr>(memorySpaceAttr);
  if (!intAttr)
    return std::nullopt;
  unsigned memorySpace = intAttr.getInt();

#define STORAGE_SPACE_MAP_FN(storage, space)                                   \
  case space:                                                                  \
    return storage;

  switch (memorySpace) {
    OPENCL_STORAGE_SPACE_MAP_LIST(STORAGE_SPACE_MAP_FN)
  default:
    break;
  }
  return std::nullopt;

#undef STORAGE_SPACE_MAP_FN
}

std::optional<unsigned>
spirv::mapOpenCLStorageClassToMemorySpace(spirv::StorageClass storageClass) {
#define STORAGE_SPACE_MAP_FN(storage, space)                                   \
  case storage:                                                                \
    return space;

  switch (storageClass) {
    OPENCL_STORAGE_SPACE_MAP_LIST(STORAGE_SPACE_MAP_FN)
  default:
    break;
  }
  return std::nullopt;

#undef STORAGE_SPACE_MAP_FN
}

#undef OPENCL_STORAGE_SPACE_MAP_LIST

//===----------------------------------------------------------------------===//
// Type Converter
//===----------------------------------------------------------------------===//

spirv::MemorySpaceToStorageClassConverter::MemorySpaceToStorageClassConverter(
    const spirv::MemorySpaceToStorageClassMap &memorySpaceMap)
    : memorySpaceMap(memorySpaceMap) {
  // Pass through for all other types.
  addConversion([](Type type) { return type; });

  addConversion([this](BaseMemRefType memRefType) -> std::optional<Type> {
    std::optional<spirv::StorageClass> storage =
        this->memorySpaceMap(memRefType.getMemorySpace());
    if (!storage) {
      LLVM_DEBUG(toolchain::dbgs()
                 << "cannot convert " << memRefType
                 << " due to being unable to find memory space in map\n");
      return std::nullopt;
    }

    auto storageAttr =
        spirv::StorageClassAttr::get(memRefType.getContext(), *storage);
    if (auto rankedType = dyn_cast<MemRefType>(memRefType)) {
      return MemRefType::get(memRefType.getShape(), memRefType.getElementType(),
                             rankedType.getLayout(), storageAttr);
    }
    return UnrankedMemRefType::get(memRefType.getElementType(), storageAttr);
  });

  addConversion([this](FunctionType type) {
    auto inputs = toolchain::map_to_vector(
        type.getInputs(), [this](Type ty) { return convertType(ty); });
    auto results = toolchain::map_to_vector(
        type.getResults(), [this](Type ty) { return convertType(ty); });
    return FunctionType::get(type.getContext(), inputs, results);
  });
}

//===----------------------------------------------------------------------===//
// Conversion Target
//===----------------------------------------------------------------------===//

/// Returns true if the given `type` is considered as legal for SPIR-V
/// conversion.
static bool isLegalType(Type type) {
  if (auto memRefType = dyn_cast<BaseMemRefType>(type)) {
    Attribute spaceAttr = memRefType.getMemorySpace();
    return isa_and_nonnull<spirv::StorageClassAttr>(spaceAttr);
  }
  return true;
}

/// Returns true if the given `attr` is considered as legal for SPIR-V
/// conversion.
static bool isLegalAttr(Attribute attr) {
  if (auto typeAttr = dyn_cast<TypeAttr>(attr))
    return isLegalType(typeAttr.getValue());
  return true;
}

/// Returns true if the given `op` is considered as legal for SPIR-V conversion.
static bool isLegalOp(Operation *op) {
  if (auto funcOp = dyn_cast<FunctionOpInterface>(op)) {
    return toolchain::all_of(funcOp.getArgumentTypes(), isLegalType) &&
           toolchain::all_of(funcOp.getResultTypes(), isLegalType) &&
           toolchain::all_of(funcOp.getFunctionBody().getArgumentTypes(),
                        isLegalType);
  }

  auto attrs = toolchain::map_range(op->getAttrs(), [](const NamedAttribute &attr) {
    return attr.getValue();
  });

  return toolchain::all_of(op->getOperandTypes(), isLegalType) &&
         toolchain::all_of(op->getResultTypes(), isLegalType) &&
         toolchain::all_of(attrs, isLegalAttr);
}

std::unique_ptr<ConversionTarget>
spirv::getMemorySpaceToStorageClassTarget(MLIRContext &context) {
  auto target = std::make_unique<ConversionTarget>(context);
  target->markUnknownOpDynamicallyLegal(isLegalOp);
  return target;
}

void spirv::convertMemRefTypesAndAttrs(
    Operation *op, MemorySpaceToStorageClassConverter &typeConverter) {
  AttrTypeReplacer replacer;
  replacer.addReplacement([&typeConverter](BaseMemRefType origType)
                              -> std::optional<BaseMemRefType> {
    return typeConverter.convertType<BaseMemRefType>(origType);
  });

  replacer.recursivelyReplaceElementsIn(op, /*replaceAttrs=*/true,
                                        /*replaceLocs=*/false,
                                        /*replaceTypes=*/true);
}

//===----------------------------------------------------------------------===//
// Conversion Pass
//===----------------------------------------------------------------------===//

namespace {
class MapMemRefStorageClassPass final
    : public impl::MapMemRefStorageClassBase<MapMemRefStorageClassPass> {
public:
  MapMemRefStorageClassPass() = default;

  explicit MapMemRefStorageClassPass(
      const spirv::MemorySpaceToStorageClassMap &memorySpaceMap)
      : memorySpaceMap(memorySpaceMap) {}

  LogicalResult initializeOptions(
      StringRef options,
      function_ref<LogicalResult(const Twine &)> errorHandler) override {
    if (failed(Pass::initializeOptions(options, errorHandler)))
      return failure();

    if (clientAPI == "opencl")
      memorySpaceMap = spirv::mapMemorySpaceToOpenCLStorageClass;
    else if (clientAPI != "vulkan")
      return errorHandler(toolchain::Twine("Invalid clienAPI: ") + clientAPI);

    return success();
  }

  void runOnOperation() override {
    MLIRContext *context = &getContext();
    Operation *op = getOperation();

    spirv::MemorySpaceToStorageClassMap spaceToStorage = memorySpaceMap;
    if (spirv::TargetEnvAttr attr = spirv::lookupTargetEnv(op)) {
      spirv::TargetEnv targetEnv(attr);
      if (targetEnv.allows(spirv::Capability::Kernel)) {
        spaceToStorage = spirv::mapMemorySpaceToOpenCLStorageClass;
      } else if (targetEnv.allows(spirv::Capability::Shader)) {
        spaceToStorage = spirv::mapMemorySpaceToVulkanStorageClass;
      }
    }

    spirv::MemorySpaceToStorageClassConverter converter(spaceToStorage);
    // Perform the replacement.
    spirv::convertMemRefTypesAndAttrs(op, converter);

    // Check if there are any illegal ops remaining.
    std::unique_ptr<ConversionTarget> target =
        spirv::getMemorySpaceToStorageClassTarget(*context);
    op->walk([&target, this](Operation *childOp) {
      if (target->isIllegal(childOp)) {
        childOp->emitOpError("failed to legalize memory space");
        signalPassFailure();
        return WalkResult::interrupt();
      }
      return WalkResult::advance();
    });
  }

private:
  spirv::MemorySpaceToStorageClassMap memorySpaceMap =
      spirv::mapMemorySpaceToVulkanStorageClass;
};
} // namespace

std::unique_ptr<OperationPass<>> mlir::createMapMemRefStorageClassPass() {
  return std::make_unique<MapMemRefStorageClassPass>();
}
