//===-- CGOps.h -------------------------------------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef OPTIMIZER_DIALECT_FIRCG_CGOPS_H
#define OPTIMIZER_DIALECT_FIRCG_CGOPS_H

#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#define GET_OP_CLASSES
#include "language/Compability/Optimizer/Dialect/FIRCG/CGOps.h.inc"

#endif // OPTIMIZER_DIALECT_FIRCG_CGOPS_H
