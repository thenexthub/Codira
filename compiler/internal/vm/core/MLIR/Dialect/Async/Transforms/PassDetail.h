//===- PassDetail.h - Async Pass class details ------------------*- C++ -*-===//
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

#ifndef DIALECT_ASYNC_TRANSFORMS_PASSDETAIL_H_
#define DIALECT_ASYNC_TRANSFORMS_PASSDETAIL_H_

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Pass/Pass.h"

namespace mlir {

namespace arith {
class ArithDialect;
} // namespace arith

namespace async {
class AsyncDialect;
} // namespace async

namespace scf {
class SCFDialect;
} // namespace scf

// -------------------------------------------------------------------------- //
// Utility functions shared by Async Transformations.
// -------------------------------------------------------------------------- //

// Forward declarations.
class OpBuilder;

namespace async {

/// Clone ConstantLike operations that are defined above the given region and
/// have users in the region into the region entry block. We do that to reduce
/// the number of function arguments when we outline `async.execute` and
/// `scf.parallel` operations body into functions.
void cloneConstantsIntoTheRegion(Region &region);
void cloneConstantsIntoTheRegion(Region &region, OpBuilder &builder);

} // namespace async

} // namespace mlir

#endif // DIALECT_ASYNC_TRANSFORMS_PASSDETAIL_H_
