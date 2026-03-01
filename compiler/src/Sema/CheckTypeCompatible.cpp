/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file implements checking of substitutability of two types.
 */
#include "TypeCheckUtil.h"

namespace Codira::TypeCheckUtil {
using namespace AST;

ComparisonRes CompareIntAndFloat(const Ty& left, const Ty& right)
{
    if (left.kind == right.kind) {
        return ComparisonRes::EQ;
    }
    if (left.IsInteger()) {
        if (right.IsInteger()) {
            if (left.kind == TypeKind::TYPE_INT64) {
                return ComparisonRes::LT;
            }
            if (right.kind == TypeKind::TYPE_INT64) {
                return ComparisonRes::GT;
            }
        } else {
            // right is float.
            return ComparisonRes::LT;
        }
    } else {
        // left is float.
        if (right.IsInteger()) {
            return ComparisonRes::GT;
        }
        if (left.kind == TypeKind::TYPE_FLOAT64) {
            return ComparisonRes::LT;
        }
        if (right.kind == TypeKind::TYPE_FLOAT64) {
            return ComparisonRes::GT;
        }
    }
    return ComparisonRes::EQ;
}
} // namespace Codira::TypeCheckUtil
