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
desc: postfix operators with other lower precedence operators
---*/

function main() {

    	let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255 = 255

	// unary plus
	res = + t2++
	arktest.assertEQ(res, 2)
	arktest.assertEQ(t2, 3)
	t2 = 2

	res = + t2--
	arktest.assertEQ(res, 2)
	arktest.assertEQ(t2, 1)
	t2 = 2

	// unary minus
	res = - t2++
	arktest.assertEQ(res, -2)
	arktest.assertEQ(t2, 3)
	t2 = 2

	res = - t2--
	arktest.assertEQ(res, -2)
	arktest.assertEQ(t2, 1)
	t2 = 2

	// bitwise complement
	res = ~ t2++
	arktest.assertEQ(res, -3)
	arktest.assertEQ(t2, 3)
	t2 = 2

	res = ~ t2--
	arktest.assertEQ(res, -3)
	arktest.assertEQ(t2, 1)
	t2 = 2

	// multiplicative
	res = t4-- * t2++
	arktest.assertEQ(res, 8)
	arktest.assertEQ(t2, 3)
	arktest.assertEQ(t4, 3)
	t2 = 2
	t4 = 4

	res = t4++ * t2--
	arktest.assertEQ(res, 8)
	arktest.assertEQ(t2, 1)
	arktest.assertEQ(t4, 5)
	t2 = 2
	t4 = 4

	// division
	res = t4-- / t2++
	arktest.assertEQ(res, 2)
	arktest.assertEQ(t2, 3)
	arktest.assertEQ(t4, 3)
	t2 = 2
	t4 = 4

	res = t4++ / t2--
	arktest.assertEQ(res, 2)
	arktest.assertEQ(t2, 1)
	arktest.assertEQ(t4, 5)
	t2 = 2
	t4 = 4

	// modulo
	res = t255-- % t4++
	arktest.assertEQ(res, 3)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255++ % t4--
	arktest.assertEQ(res, 3)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// addition
	res = t255-- + t4++
	arktest.assertEQ(res, 259)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255++ + t4--
	arktest.assertEQ(res, 259)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// subtraction
	res = t255-- - t4++
	arktest.assertEQ(res, 251)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255++ - t4--
	arktest.assertEQ(res, 251)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// casting
	res = (t255--).toByte() - (t4++).toByte()
	arktest.assertEQ(res, -5)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = (t255++).toByte() - (t4--).toByte()
	arktest.assertEQ(res, -5)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// left shift
	res = t255-- << t4++
	arktest.assertEQ(res, 4080)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255++ << t4--
	arktest.assertEQ(res, 4080)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// right shift
	res = t255-- >> t4++
	arktest.assertEQ(res, 15)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255++ >> t4--
	arktest.assertEQ(res, 15)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// right no-sign shift
	res = - t255-- >>> t4++
	arktest.assertEQ(res, 268435440)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = - t255++ >>> t4--
	arktest.assertEQ(res, 268435440)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// relational
	arktest.assertFalse(t3-- < t2++)
	t3 = 3
	t2 = 2

	arktest.assertFalse(t8-- <= t4++)
	t8 = 8
	t4 = 4

	arktest.assertFalse(t2++ > t3--)
	t3 = 3
	t2 = 2

	arktest.assertFalse(t4++ >= t8--)
	t8 = 8
	t4 = 4

	// equality
	arktest.assertFalse(t3-- == t2)
	t3 = 3

	arktest.assertFalse(t3++ == t4)
	t3 = 3

	arktest.assertFalse(t3-- != 3)
	t3 = 3

	arktest.assertFalse(t3++ != 3)
	t3 = 3

	// bitwise AND
	res = t255-- & t4++
	arktest.assertEQ(res, 4)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255++ & t4--
	arktest.assertEQ(res, 4)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// bitwise OR
	res = t255-- | t4++
	arktest.assertEQ(res, 255)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255++ | t4--
	arktest.assertEQ(res, 255)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// bitwise XOR
	res = t255-- ^ t4++
	arktest.assertEQ(res, 251)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255++ ^ t4--
	arktest.assertEQ(res, 251)
	arktest.assertEQ(t255, 256)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	// ternary
	res = t8 > t4 ? t255-- : t4++
	arktest.assertEQ(res, 255)
	arktest.assertEQ(t255, 254)
	arktest.assertEQ(t4, 4)
	t255 = 255

	res = t8 > t4 ? t4-- : t255++
	arktest.assertEQ(res, 4)
	arktest.assertEQ(t255, 255)
	arktest.assertEQ(t4, 3)
	t4 = 4

	// assignment
	res = t255 = t3--
	arktest.assertEQ(res, 3)
	arktest.assertEQ(t255, 3)
	arktest.assertEQ(t3, 2)
	t255 = 255
	t3 = 3

	res = t255 = t3++
	arktest.assertEQ(res, 3)
	arktest.assertEQ(t255, 3)
	arktest.assertEQ(t3, 4)
	t255 = 255
	t3 = 3

	res = t255 += t3--
	arktest.assertEQ(res, 258)
	arktest.assertEQ(t255, 258)
	arktest.assertEQ(t3, 2)
	t255 = 255
	t3 = 3

	res = t255 += t3++
	arktest.assertEQ(res, 258)
	arktest.assertEQ(t255, 258)
	arktest.assertEQ(t3, 4)
	t255 = 255
	t3 = 3

	res = t255 -= t3--
	arktest.assertEQ(res, 252)
	arktest.assertEQ(t255, 252)
	arktest.assertEQ(t3, 2)
	t255 = 255
	t3 = 3

	res = t255 -= t3++
	arktest.assertEQ(res, 252)
	arktest.assertEQ(t255, 252)
	arktest.assertEQ(t3, 4)
	t255 = 255
	t3 = 3

	res = t255 *= t3--
	arktest.assertEQ(res, 765)
	arktest.assertEQ(t255, 765)
	arktest.assertEQ(t3, 2)
	t255 = 255
	t3 = 3

	res = t255 *= t3++
	arktest.assertEQ(res, 765)
	arktest.assertEQ(t255, 765)
	arktest.assertEQ(t3, 4)
	t255 = 255
	t3 = 3

	res = t255 /= t3--
	arktest.assertEQ(res, 85)
	arktest.assertEQ(t255, 85)
	arktest.assertEQ(t3, 2)
	t255 = 255
	t3 = 3

	res = t255 /= t3++
	arktest.assertEQ(res, 85)
	arktest.assertEQ(t255, 85)
	arktest.assertEQ(t3, 4)
	t255 = 255
	t3 = 3

	res = t255 %= t4--
	arktest.assertEQ(res, 3)
	arktest.assertEQ(t255, 3)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	res = t255 %= t4++
	arktest.assertEQ(res, 3)
	arktest.assertEQ(t255, 3)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255 <<= t4--
	arktest.assertEQ(res, 4080)
	arktest.assertEQ(t255, 4080)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	res = t255 <<= t4++
	arktest.assertEQ(res, 4080)
	arktest.assertEQ(t255, 4080)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255 >>= t4--
	arktest.assertEQ(res, 15)
	arktest.assertEQ(t255, 15)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	res = t255 >>= t4++
	arktest.assertEQ(res, 15)
	arktest.assertEQ(t255, 15)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255 >>>= t4--
	arktest.assertEQ(res, 15)
	arktest.assertEQ(t255, 15)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	res = t255 >>>= t4++
	arktest.assertEQ(res, 15)
	arktest.assertEQ(t255, 15)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255 &= t4--
	arktest.assertEQ(res, 4)
	arktest.assertEQ(t255, 4)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	res = t255 &= t4++
	arktest.assertEQ(res, 4)
	arktest.assertEQ(t255, 4)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255 |= t4--
	arktest.assertEQ(res, 255)
	arktest.assertEQ(t255, 255)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	res = t255 |= t4++
	arktest.assertEQ(res, 255)
	arktest.assertEQ(t255, 255)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4

	res = t255 ^= t4--
	arktest.assertEQ(res, 251)
	arktest.assertEQ(t255, 251)
	arktest.assertEQ(t4, 3)
	t255 = 255
	t4 = 4

	res = t255 ^= t4++
	arktest.assertEQ(res, 251)
	arktest.assertEQ(t255, 251)
	arktest.assertEQ(t4, 5)
	t255 = 255
	t4 = 4
}
