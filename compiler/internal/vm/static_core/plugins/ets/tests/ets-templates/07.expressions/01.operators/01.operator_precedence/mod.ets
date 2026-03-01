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
desc: remainder operator with other operators
---*/

function main() {

    	let res: int, t2 = 2, t3 = 3, t4 = 4, t7 = 7, t8 = 8, t255 = 255, t518 = 518;

	// multiplication
	res = t3 * t7 % t3
	arktest.assertEQ(res, 0)

	// division
	res = t8 / t3 % t2
	arktest.assertEQ(res, 0)

	// addition
	res = t2 + t7 % t2
	arktest.assertEQ(res, 3)

	// subtraction
	res = t8 - t7 % t4
	arktest.assertEQ(res, 5)

	// modulo
	res = (t518 % t255).toByte()
	arktest.assertEQ(res, 8)

	// cast
	res = t518.toByte() % t255
	arktest.assertEQ(res, 6)

	// shifts
	res = t4 << t3 % t2
	arktest.assertEQ(res, 8)

	res = t255 >> t3 % t2
	arktest.assertEQ(res, 127)

	res = - t255 >>> t7 % t4
	arktest.assertEQ(res, 536870880)

	// relational
	arktest.assertFalse(t8 < t518 % t255)
	arktest.assertFalse(t255 <= t518 % t255)
	arktest.assertFalse(t3 > t7 % t4)
	arktest.assertFalse(t2 >= t7 % t4)
	arktest.assertFalse(t4 == t8 % t3)
	arktest.assertFalse(t2 != t8 % t3)

	// bitwise
	res = t7 & t255 % t7
	arktest.assertEQ(res, 3)

	res = t8 | t3 % t2
	arktest.assertEQ(res, 9)

	res = t255 ^ t7 % t2
	arktest.assertEQ(res, 254)

	// ternary
	res = t8 > t2 ? t4 : t8 % t4
	arktest.assertEQ(res, 4)

	res = t7 % t4 > t2 ? t2 : t8
	arktest.assertEQ(res, 2)

	// assignments
	res = t2 += t7 % t3
	arktest.assertEQ(res, 3)
	t2 = 2

	res = t2 -= t7 % t3
	arktest.assertEQ(res, 1)
	t2 = 2

	res = t8 *= t7 % t3
	arktest.assertEQ(res, 8)
	t8 = 8

	res = t8 /= t7 % t2
	arktest.assertEQ(res, 8)
	t8 = 8

	res = t3 &= t7 % t3
	arktest.assertEQ(res, 1)
	t3 = 3

	res = t2 |= t7 % t3
	arktest.assertEQ(res, 3)
	t2 = 2

	res = t255 ^= t7 % t4
	arktest.assertEQ(res, 252)
	t255 = 255

	res = t4 <<= t3 % t2
	arktest.assertEQ(res, 8)
	t4 = 4

	res = t255 >>= t3 % t2
	arktest.assertEQ(res, 127)
	t255 = 255

	res = t255 >>>= t3 % t2
	arktest.assertEQ(res, 127)
	t255 = 255
    
}
