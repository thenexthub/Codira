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
desc: subtraction operator with other lower precedence operator
---*/

function main() {

 	let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255 = 255

	// cast
	res = (t2 - t255).toByte()
	arktest.assertEQ(res, 3)

	res = t255.toByte() - t3
	arktest.assertEQ(res, -4)

	// shifts
	res = t4 << t3 - t2
	arktest.assertEQ(res, 8)

	res = - t255 >> t3 - t2
	arktest.assertEQ(res, -128)

	res = - t255 >>> t8 - t3
	arktest.assertEQ(res, 134217720)

	// relational
	arktest.assertFalse(t4 < t8 - t4)
	arktest.assertFalse(t4 <= t4 - t3)
	arktest.assertFalse(t2 > t4 - t2)
	arktest.assertFalse(t4 >= t8 - t2)
	arktest.assertFalse(t2 == t2 - t4)
	arktest.assertFalse(t4 != t8 - t4)

	// bitwise
	res = t255 & t4 - t8
	arktest.assertEQ(res, 252)

	res = t3 | t4 - t2
	arktest.assertEQ(res, 3)

	res = t255 ^ t8 - t2
	arktest.assertEQ(res, 249)

	// ternary
	res = t3 <= t4 ? t4 : t3 - t4
	arktest.assertEQ(res, 4)

	res = t4 - t2 > t4 ? t4 : t3
	arktest.assertEQ(res, 3)

	// assignments
	res = t4 += t4 - t3
	arktest.assertEQ(res, 5)
	t4 = 4

	res = t4 -= t4 - t3
	arktest.assertEQ(res, 3)
	t4 = 4

	res = t8 *= t2 - t3
	arktest.assertEQ(res, -8)
	t8 = 8

	res = t8 /= t4 - t2
	arktest.assertEQ(res, 4)
	t8 = 8

	res = t8 %= t3 - t2
	arktest.assertEQ(res, 0)
	t8 = 8

	res = t255 &= t2 - t3
	arktest.assertEQ(res, 255)
	t255 = 255

	res = t2 |= t3 - t4
	arktest.assertEQ(res, -1)
	t2 = 2

	res = t255 ^= t4 - t8
	arktest.assertEQ(res, -253)
	t255 = 255

	res = t4 <<= t3 - t2
	arktest.assertEQ(res, 8)
	t4 = 4

	res = t255 >>= t4 - t2
	arktest.assertEQ(res, 63)
	t255 = 255

	res = t255 >>>= t4 - t2
	arktest.assertEQ(res, 63)
	t255 = 255
    
}
