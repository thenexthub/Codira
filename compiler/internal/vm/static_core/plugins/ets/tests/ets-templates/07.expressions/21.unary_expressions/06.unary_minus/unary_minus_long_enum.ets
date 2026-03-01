/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: >-
    Type of the operand expression with the unary operator -
    must be convertible (see Implicit Conversions)
    to a numeric type (see Numeric Types)
---*/


enum LongEnum {
    Neg = -0x1_1111_1111,
    Pos = 0x1_1111_1111,
    Min = -9223372036854775808,
    Max = 9223372036854775807
}

function main(): void {
    let n = -LongEnum.Neg
    arktest.assertEQ(n, 0x1_1111_1111)
    arktest.assertEQ(-n, -0x1_1111_1111)

    let p = -LongEnum.Pos
    arktest.assertEQ(p, -0x1_1111_1111)
    arktest.assertEQ(-p, 0x1_1111_1111)

    arktest.assertEQ(-LongEnum.Max, -9223372036854775807)

    arktest.assertEQ(-LongEnum.Neg, 0x1_1111_1111)
    arktest.assertEQ(-LongEnum.Pos, -0x1_1111_1111)
}
