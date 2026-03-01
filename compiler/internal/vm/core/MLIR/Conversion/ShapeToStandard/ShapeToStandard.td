//==-- ShapeToStandard.td - Shape to Standard Patterns -------*- tablegen -*==//
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
// Defines Patterns to lower Shape ops to Std.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_CONVERSION_SHAPETOSTANDARD_TD
#define MLIR_CONVERSION_SHAPETOSTANDARD_TD

include "mlir/IR/PatternBase.td"
include "mlir/Dialect/Shape/IR/ShapeOps.td"

def BroadcastableStringAttr : NativeCodeCall<[{
  $_builder.getStringAttr("required broadcastable shapes")
}]>;

def CstrBroadcastableToRequire : Pat<(Shape_CstrBroadcastableOp $shapes),
            (Shape_CstrRequireOp
              (Shape_IsBroadcastableOp $shapes),
              (BroadcastableStringAttr))>;

def EqStringAttr : NativeCodeCall<[{
  $_builder.getStringAttr("required equal shapes")
}]>;

def CstrEqToRequire : Pat<(Shape_CstrEqOp $shapes),
  (Shape_CstrRequireOp (Shape_ShapeEqOp $shapes), (EqStringAttr))>;

#endif // MLIR_CONVERSION_SHAPETOSTANDARD_TD
