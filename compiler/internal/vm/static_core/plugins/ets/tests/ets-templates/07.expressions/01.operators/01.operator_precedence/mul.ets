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
desc: multiplicative operator with other operators
---*/

function main() {

    	let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255 = 255

	// division
	res = t8 / t3 * t2
	arktest.assertEQ(res, 4)

	// modulo
	res = t8 % t3 * t2
	arktest.assertEQ(res, 4)

	// addition
	res = t2 + t4 * t3
	arktest.assertEQ(res, 14)

	// subtraction
	res = t2 - t3 * t4
	arktest.assertEQ(res, -10)

	// cast
	res = t2 * t255.toByte()
	arktest.assertEQ(res, -2)

	res = t255.toByte() * t3
	arktest.assertEQ(res, -3)

	// shifts
	res = t4 << t3 * t2
	arktest.assertEQ(res, 256)

	res = - t255 >> t2 * t3
	arktest.assertEQ(res, -4)

	res = - t255 >>> t2 * t3
	arktest.assertEQ(res, 67108860)

	// relational
	arktest.assertFalse(t255 < t8 * t8)
	arktest.assertFalse(t255 <= t2 * t8)
	arktest.assertFalse(t2 > t8 * t4)
	arktest.assertFalse(t4 >= t8 * t4)
	arktest.assertFalse(t2 == t2 * t4)
	arktest.assertFalse(t8 != t4 * t2)

	// bitwise
	res = t255 & t4 * t8
	arktest.assertEQ(res, 32)

	res = t3 | t4 * t2
	arktest.assertEQ(res, 11)

	res = t255 ^ t8 * t2
	arktest.assertEQ(res, 239)

	// ternary
	res = t3 <= t4 ? t4 : t3 * t4
	arktest.assertEQ(res, 4)

	res = t4 * t2 > t4 ? t4 : t3
	arktest.assertEQ(res, 4)

	// assignments
	res = t2 += t4 * t3
	arktest.assertEQ(res, 14)
	t2 = 2

	res = t2 -= t4 * t3
	arktest.assertEQ(res, -10)
	t2 = 2

	res = t8 /= t2 * t3
	arktest.assertEQ(res, 1)
	t8 = 8

	res = t8 %= t2 * t3
	arktest.assertEQ(res, 2)
	t8 = 8

	res = t255 &= t2 * t3
	arktest.assertEQ(res, 6)
	t255 = 255

	res = t2 |= t3 * t4
	arktest.assertEQ(res, 14)
	t2 = 2

	res = t255 ^= t4 * t8
	arktest.assertEQ(res, 223)
	t255 = 255

	res = t4 <<= t3 * t2
	arktest.assertEQ(res, 256)
	t4 = 4

	res = t255 >>= t2 * t3
	arktest.assertEQ(res, 3)
	t255 = 255

	res = t255 >>>= t3 * t2
	arktest.assertEQ(res, 3)
	t255 = 255
    
}
