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

/*---
desc: >-
    Type of the operand expression with the unary operator +
    must be convertible (see Implicit Conversions)
    to a numeric type (see Numeric Types)
---*/

enum IntEnum {
    Neg = -1,
    Pos = 1,
    Min = -2147483648,
    Max = 2147483647
}

function main(): void {
    let n = +IntEnum.Neg
    arktest.assertEQ(n, -1)
    arktest.assertEQ(+n, -1)

    let p = +IntEnum.Pos
    arktest.assertEQ(p, 1)
    arktest.assertEQ(+p, 1)

    arktest.assertEQ(+IntEnum.Neg, IntEnum.Neg)
    arktest.assertEQ(+IntEnum.Pos, IntEnum.Pos)

    arktest.assertEQ(+IntEnum.Min, IntEnum.Min)
    arktest.assertEQ(+IntEnum.Max, IntEnum.Max)
}
