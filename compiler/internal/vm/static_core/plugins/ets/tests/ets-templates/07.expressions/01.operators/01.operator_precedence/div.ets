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
desc: division operator with other operators
---*/

function main() {

    	let res: int, t2 = 2, t3 = 3, t4 = 4, t7 = 7, t8 = 8, t255 = 255, t510 = 510;

	// multiplication
	res = t8 * t3 / t2
	arktest.assertEQ(res, 12)

	// modulo
	res = t8 % t3 / t2
	arktest.assertEQ(res, 1)

	// addition
	res = t2 + t4 / t2
	arktest.assertEQ(res, 4)

	// subtraction
	res = t2 - t8 / t4
	arktest.assertEQ(res, 0)

	// cast
	res = (t510 / t255).toByte()
	arktest.assertEQ(res, 2)

	res = t510.toByte() / t255
	arktest.assertEQ(res, 0)

	// shifts
	res = t4 << t4 / t2
	arktest.assertEQ(res, 16)

	res = t255 >> t4 / t2
	arktest.assertEQ(res, 63)

	res = - t255 >>> t8 / t2
	arktest.assertEQ(res, 268435440)

	// relational
	arktest.assertFalse(t255 < t510 / t2)
	arktest.assertFalse(t255 <= t510 / t255)
	arktest.assertFalse(t2 > t8 / t4)
	arktest.assertFalse(t4 >= t510 / t4)
	arktest.assertFalse(t2 == t8 / t2)
	arktest.assertFalse(t4 != t8 / t2)

	// bitwise
	res = t3 & t7 / t2
	arktest.assertEQ(res, 3)

	res = t255 | t7 / t2
	arktest.assertEQ(res, 255)

	res = t255 ^ t8 / t2
	arktest.assertEQ(res, 251)

	// ternary
	res = t8 > t2 ? t4 : t8 / t4
	arktest.assertEQ(res, 4)

	res = t8 / t4 > t2 ? t4 : t8
	arktest.assertEQ(res, 8)

	// assignments
	res = t2 += t8 / t2
	arktest.assertEQ(res, 6)
	t2 = 2

	res = t2 -= t4 / t2
	arktest.assertEQ(res, 0)
	t2 = 2

	res = t8 *= t7 / t2
	arktest.assertEQ(res, 24)
	t8 = 8

	res = t8 %= t7 / t2
	arktest.assertEQ(res, 2)
	t8 = 8

	res = t7 &= t255 / t2
	arktest.assertEQ(res, 7)
	t7 = 7

	res = t2 |= t7 / t4
	arktest.assertEQ(res, 3)
	t2 = 2

	res = t255 ^= t8 / t4
	arktest.assertEQ(res, 253)
	t255 = 255

	res = t4 <<= t3 / t2
	arktest.assertEQ(res, 8)
	t4 = 4

	res = t255 >>= t3 / t2
	arktest.assertEQ(res, 127)
	t255 = 255

	res = t255 >>>= t3 / t2
	arktest.assertEQ(res, 127)
	t255 = 255
}
