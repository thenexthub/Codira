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
desc: bitwise operators with other operators
---*/

function main() {

    let res: int, t2 = 2, t3 = 3, t4 = 4, t7 = 7, t8 = 8, t255 = 255

    // bitwise
    res = t3 | t4 & t8
    arktest.assertEQ(res, 3)

    res = t3 ^ t4 & t2
    arktest.assertEQ(res, 3)

    res = t3 | t4 ^ t2
    arktest.assertEQ(res, 7)

    // ternary
    res = t3 <= t4 ? t7 : t2 & t3
    arktest.assertEQ(res, 7)

    res = t3 <= t4 ? t7 : t2 ^ t3
    arktest.assertEQ(res, 7)

    res = t3 <= t4 ? t7 : t2 | t255
    arktest.assertEQ(res, 7)

    // assignments
    res = t2 += t7 & t3
    arktest.assertEQ(res, 5)
    t2 = 2

    res = t2 += t7 ^ t3
    arktest.assertEQ(res, 6)
    t2 = 2

    res = t2 += t7 | t3
    arktest.assertEQ(res, 9)
    t2 = 2

    res = t2 -= t7 & t3
    arktest.assertEQ(res, -1)
    t2 = 2

    res = t2 -= t7 ^ t3
    arktest.assertEQ(res, -2)
    t2 = 2

    res = t2 -= t7 | t255
    arktest.assertEQ(res, -253)
    t2 = 2

    res = t2 *= t7 & t3
    arktest.assertEQ(res, 6)
    t2 = 2

    res = t2 *= t7 ^ t3
    arktest.assertEQ(res, 8)
    t2 = 2

    res = t2 *= t7 | t255
    arktest.assertEQ(res, 510)
    t2 = 2

    res = t255 /= t7 & t3
    arktest.assertEQ(res, 85)
    t255 = 255

    res = t255 /= t7 ^ t3
    arktest.assertEQ(res, 63)
    t255 = 255

    res = t255 /= t7 | t255
    arktest.assertEQ(res, 1)
    t255 = 255

    res = t255 %= t7 & t3
    arktest.assertEQ(res, 0)
    t255 = 255

    res = t255 %= t7 ^ t3
    arktest.assertEQ(res, 3)
    t255 = 255

    res = t255 %= t7 | t255
    arktest.assertEQ(res, 0)
    t255 = 255

    res = t255 &= t7 & t3
    arktest.assertEQ(res, 3)
    t255 = 255

    res = t255 &= t7 ^ t255
    arktest.assertEQ(res, 248)
    t255 = 255

    res = t255 &= t7 | t255
    arktest.assertEQ(res, 255)
    t255 = 255

    res = t255 ^= t7 & t3
    arktest.assertEQ(res, 252)
    t255 = 255

    res = t255 ^= t7 ^ t255
    arktest.assertEQ(res, 7)
    t255 = 255

    res = t255 ^= t7 | t255
    arktest.assertEQ(res, 0)
    t255 = 255

    res = t255 |= t7 & t3
    arktest.assertEQ(res, 255)
    t255 = 255

    res = t255 |= t7 ^ t255
    arktest.assertEQ(res, 255)
    t255 = 255

    res = t255 |= t7 | t255
    arktest.assertEQ(res, 255)
    t255 = 255

    res = t255 <<= t7 & t3
    arktest.assertEQ(res, 2040)
    t255 = 255

    res = t255 <<= t7 ^ t255
    arktest.assertEQ(res, -16777216)
    t255 = 255

    res = t255 <<= t7 | t255
    arktest.assertEQ(res, -2147483648)
    t255 = 255

    res = t255 >>= t7 & t3
    arktest.assertEQ(res, 31)
    t255 = 255

    res = t255 >>= t2 ^ t7
    arktest.assertEQ(res, 7)
    t255 = 255

    res = t255 >>= t7 | t7
    arktest.assertEQ(res, 1)
    t255 = 255

    res = t255 >>>= t7 & t3
    arktest.assertEQ(res, 31)
    t255 = 255

    res = t255 >>>= t2 ^ t7
    arktest.assertEQ(res, 7)
    t255 = 255

    res = t255 >>>= t7 | t7
    arktest.assertEQ(res, 1)
    t255 = 255

}
