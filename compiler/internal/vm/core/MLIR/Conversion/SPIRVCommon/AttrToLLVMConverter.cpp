//===- AttrToLLVMConverter.cpp - SPIR-V attributes conversion to LLVM -C++ ===//
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

#include <mlir/Conversion/SPIRVCommon/AttrToLLVMConverter.h>

namespace mlir {
namespace {

//===----------------------------------------------------------------------===//
// Constants
//===----------------------------------------------------------------------===//

constexpr unsigned defaultAddressSpace = 0;

//===----------------------------------------------------------------------===//
// Utility functions
//===----------------------------------------------------------------------===//

static unsigned
storageClassToOCLAddressSpace(spirv::StorageClass storageClass) {
  // Based on
  // https://registry.khronos.org/SPIR-V/specs/unified1/OpenCL.ExtendedInstructionSet.100.html#_binary_form
  // and clang/lib/Basic/Targets/SPIR.h.
  switch (storageClass) {
  case spirv::StorageClass::Function:
    return 0;
  case spirv::StorageClass::Input:
  case spirv::StorageClass::CrossWorkgroup:
    return 1;
  case spirv::StorageClass::UniformConstant:
    return 2;
  case spirv::StorageClass::Workgroup:
    return 3;
  case spirv::StorageClass::Generic:
    return 4;
  case spirv::StorageClass::DeviceOnlyINTEL:
    return 5;
  case spirv::StorageClass::HostOnlyINTEL:
    return 6;
  default:
    return defaultAddressSpace;
  }
}
} // namespace

unsigned storageClassToAddressSpace(spirv::ClientAPI clientAPI,
                                    spirv::StorageClass storageClass) {
  switch (clientAPI) {
  case spirv::ClientAPI::OpenCL:
    return storageClassToOCLAddressSpace(storageClass);
  default:
    return defaultAddressSpace;
  }
}
} // namespace mlir
