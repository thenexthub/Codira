/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: additive operator with other lower precedence operators
---*/

function main() {

    let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255  = 255

    // cast
    res = (t2 + t255).toByte()
    arktest.assertEQ(res, 1)

    res = t255.toByte() + t3
    arktest.assertEQ(res, 2)

    // shifts
    res = t4 << t3 + t2
    arktest.assertEQ(res, 128)

    res = - t255 >> t2 + t3
    arktest.assertEQ(res, -8)

    res = - t255 >>> t2 + t3
    arktest.assertEQ(res, 134217720)

    // relational
    arktest.assertFalse(t8 < t4 + t4)
    arktest.assertFalse(t255 <= t8 + t4)
    arktest.assertFalse(t2 > t2 + t4)
    arktest.assertFalse(t4 >= t2 + t4)
    arktest.assertFalse(t2 == t2 + t4)
    arktest.assertFalse(t8 != t4 + t4)

    // bitwise
    res = t255 & t4 + t8
    arktest.assertEQ(res, 12)

    res = t3 | t4 + t2
    arktest.assertEQ(res, 7)

    res = t255 ^ t8 + t2
    arktest.assertEQ(res, 245)

    // ternary
    res = t3 <= t4 ? t4 : t3 + t4
    arktest.assertEQ(res, 4)

    res = t4 + t2 > t4 ? t4 : t3
    arktest.assertEQ(res, 4)

    // assignments
    res = t2 += t4 + t3
    arktest.assertEQ(res, 9)
    t2 = 2

    res = t2 -= t4 + t3
    arktest.assertEQ(res, -5) 
    t2 = 2;

    res = t8 *= t2 + t3
    arktest.assertEQ(res, 40) 
    t8 = 8;

    res = t8 /= t2 + t3
    arktest.assertEQ(res, 1) 
    t8 = 8;

    res = t8 %= t2 + t3
    arktest.assertEQ(res, 3) 
    t8 = 8;

    res = t255 &= t2 + t3
    arktest.assertEQ(res, 5) 
    t255 = 255;

    res = t2 |= t3 + t4
    arktest.assertEQ(res, 7) 
    t2 = 2;

    res = t255 ^= t4 + t8
    arktest.assertEQ(res, 243) 
    t255 = 255;

    res = t4 <<= t3 + t2
    arktest.assertEQ(res, 128) 
    t4 = 4;

    res = t255 >>= t2 + t3
    arktest.assertEQ(res, 7) 
    t255 = 255;

    res = t255 >>>= t3 + t2
    arktest.assertEQ(res, 7) 
    t255 = 255;
}
