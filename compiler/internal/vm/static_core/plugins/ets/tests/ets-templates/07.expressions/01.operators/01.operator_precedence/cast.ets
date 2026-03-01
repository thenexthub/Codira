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
desc: cast operator with other lower precedence operators
---*/

function main() {

    	let res: int, t1 = 1, t2 = 2, t3 = 3, t4 = 4, t255 = 255, t257 = 257, t258 = 258

	// shifts
	res = t255 << t257.toByte()
	arktest.assertEQ(res, 510)

	res = t257 >> t257.toByte()
	arktest.assertEQ(res, 128)

	res = - t255 >>> t257.toByte()
	arktest.assertEQ(res, 2147483520)

	// relational
	arktest.assertFalse(t2 < t258.toByte())
	arktest.assertFalse(t3 <= t258.toByte())
	arktest.assertFalse(t2 > t258.toByte())
	arktest.assertFalse(t1 >= t258.toByte())
	arktest.assertFalse(t1 == t258.toByte())
	arktest.assertFalse(t2 != t258.toByte())

	// bitwise
	res = t255 & t258.toByte()
	arktest.assertEQ(res, 2)

	res = t3 | t258.toByte()
	arktest.assertEQ(res, 3)

	res = t255 ^ t258.toByte()
	arktest.assertEQ(res, 253)

	// ternary
	res = t3 <= t4 ? t257 : t258.toByte()
	arktest.assertEQ(res, 257)

	// assignments
	res = t258 += t257.toByte()
	arktest.assertEQ(res, 259)
	t258 = 258

	res = t258 -= t257.toByte()
	arktest.assertEQ(res, 257)
	t258 = 258

	res = t258 *= t257.toByte()
	arktest.assertEQ(res, 258)
	t258 = 258

	res = t258 /= t257.toByte()
	arktest.assertEQ(res, 258)
	t258 = 258

	res = t255 %= t258.toByte()
	arktest.assertEQ(res, 1)
	t255 = 255

	res = t255 &= t258.toByte()
	arktest.assertEQ(res, 2)
	t255 = 255

	res = t3 |= t258.toByte()
	arktest.assertEQ(res, 3)
	t3 = 3

	res = t255 ^= t258.toByte()
	arktest.assertEQ(res, 253)
	t255 = 255

	res = t255 <<= t257.toByte()
	arktest.assertEQ(res, 510)
	t255 = 255

	res = t257 >>= t257.toByte()
	arktest.assertEQ(res, 128)
	t257 = 257

	res = t255 >>>= t257.toByte()
	arktest.assertEQ(res, 127)
	t255 = 255
    
}
