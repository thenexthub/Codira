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
desc: shift operators with other lower precedence operators
---*/

function main() {

   	let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255 = 255

	// shifts
	res = t255 << t3 >> t2 >>> t4
	arktest.assertEQ(res, 31)

	// relational
	arktest.assertFalse(t8 < t2 << t2)
	arktest.assertFalse(t255 <= t2 << t2)
	arktest.assertFalse(t8 > t2 << t2)
	arktest.assertFalse(t4 >= t2 << t2)
	arktest.assertFalse(t4 == t2 << t2)
	arktest.assertFalse(t8 != t2 << t2)

	// bitwise
	res = t4 & t4 << t2
	arktest.assertEQ(res, 0)

	res = t3 | t4 << t2
	arktest.assertEQ(res, 19)

	res = t255 ^ t8 << t2
	arktest.assertEQ(res, 223)

	// ternary
	res = t3 <= t4 ? t4 : t3 << t2
	arktest.assertEQ(res, 4)

	res = t4 << t2 > t4 ? t4 : t3
	arktest.assertEQ(res, 4)

	// assignments
	res = t2 += t4 << t2
	arktest.assertEQ(res, 18)
	t2 = 2

	res = t2 -= t4 << t2
	arktest.assertEQ(res, -14)
	t2 = 2

	res = t8 *= t2 << t3
	arktest.assertEQ(res, 128)
	t8 = 8

	res = t255 /= t2 << t3
	arktest.assertEQ(res, 15)
	t255 = 255

	res = t255 %= t2 << t3
	arktest.assertEQ(res, 15)
	t255 = 255

	res = t255 &= t255 << t3
	arktest.assertEQ(res, 248)
	t255 = 255

	res = t255 |= t255 << t3
	arktest.assertEQ(res, 2047)
	t255 = 255

	res = t255 ^= t4 << t8
	arktest.assertEQ(res, 1279)
	t255 = 255

	res = t4 <<= t3 << t2
	arktest.assertEQ(res, 16384)
	t4 = 4

	res = t255 >>= t2 << t3
	arktest.assertEQ(res, 0)
	t255 = 255

	res = t255 >>>= t3 << t2
	arktest.assertEQ(res, 0)
	t255 = 255

    
}
