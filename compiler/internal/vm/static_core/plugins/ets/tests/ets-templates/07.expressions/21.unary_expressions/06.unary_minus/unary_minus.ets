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
desc: Unary minus operator
assertion: Numeric widening occurs on the expression before a unary plus operator is applied. 
           The same maximum negative number results from the negation of the maximum negative int or long.
---*/

function main(): void {
    let b: byte = 127
    arktest.assertEQ(-b, -127)
    arktest.assertTrue(-b instanceof int)

    let b2: byte = -128
    arktest.assertEQ(-b2, 128)
    arktest.assertTrue(-b2 instanceof int)

    let s: short = 32767
    arktest.assertEQ(-s, -32767)
    arktest.assertTrue(-s instanceof int)

    let s2: short = -32768
    arktest.assertEQ(-s2, 32768)
    arktest.assertTrue(-s2 instanceof int)

    let i: int = 2147483647
    arktest.assertEQ(-i, -2147483647)
    arktest.assertTrue(-i instanceof int)

    let i2: int = -2147483648
    arktest.assertEQ(-i2, -2147483648)
    arktest.assertTrue(-i2 instanceof int)

    let l: long = 9223372036854775807
    arktest.assertEQ(-l, -9223372036854775807);
    arktest.assertTrue(-l instanceof long)

    let l2: long = -9223372036854775808
    arktest.assertEQ(-l2, -9223372036854775808);
    arktest.assertTrue(-l2 instanceof long)

    let bi: bigint = 42n
    arktest.assertEQ(-bi, -42n);
    arktest.assertTrue(-bi instanceof bigint)

    let n: number = 3.14;
    arktest.assertEQ(-n, -3.14);
    arktest.assertTrue(-n instanceof double)

    let d: double = -3.14;
    arktest.assertEQ(-d, 3.14);
    arktest.assertTrue(-d instanceof double)

    let f: float = 3.14f;
    arktest.assertEQ(-f, -3.14f);
    arktest.assertTrue(-f instanceof float)
}
