//===-- RegisterOpenACCExtensions.cpp -------------------------------------===//
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
//
// Registration for OpenACC extensions as applied to FIR dialect.
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/OpenACC/Support/RegisterOpenACCExtensions.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/OpenACC/Support/FIROpenACCTypeInterfaces.h"

namespace fir::acc {
void registerOpenACCExtensions(mlir::DialectRegistry &registry) {
  registry.addExtension(+[](mlir::MLIRContext *ctx,
                            fir::FIROpsDialect *dialect) {
    fir::BoxType::attachInterface<OpenACCMappableModel<fir::BaseBoxType>>(*ctx);
    fir::ClassType::attachInterface<OpenACCMappableModel<fir::BaseBoxType>>(
        *ctx);
    fir::ReferenceType::attachInterface<
        OpenACCMappableModel<fir::ReferenceType>>(*ctx);
    fir::PointerType::attachInterface<OpenACCMappableModel<fir::PointerType>>(
        *ctx);
    fir::HeapType::attachInterface<OpenACCMappableModel<fir::HeapType>>(*ctx);

    fir::ReferenceType::attachInterface<
        OpenACCPointerLikeModel<fir::ReferenceType>>(*ctx);
    fir::PointerType::attachInterface<
        OpenACCPointerLikeModel<fir::PointerType>>(*ctx);
    fir::HeapType::attachInterface<OpenACCPointerLikeModel<fir::HeapType>>(
        *ctx);

    fir::LLVMPointerType::attachInterface<
        OpenACCPointerLikeModel<fir::LLVMPointerType>>(*ctx);
  });
  registerAttrsExtensions(registry);
}

} // namespace fir::acc
