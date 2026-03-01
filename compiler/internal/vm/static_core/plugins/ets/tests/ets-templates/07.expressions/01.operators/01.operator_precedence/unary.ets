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
desc: unary minus, plus, and bitwise complement operators with other lower precedence operators
---*/

function main() {
	let i: int, b: boolean, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255 = 255, t258 = 258

	// multiplication
	i = + t2 * t4; arktest.assertEQ(i, 8)
	i = + + t2 * t4; arktest.assertEQ(i, 8)
	i = - t2 * t4; arktest.assertEQ(i, -8)
	i = - - t2 * t4; arktest.assertEQ(i, 8)
	i = ~ t2 * t4; arktest.assertEQ(i, -12)
	i = ~ ~ t2 * t4; arktest.assertEQ(i, 8)
	i = + - t2 * t4; arktest.assertEQ(i, -8)
	i = + ~ t2 * t4; arktest.assertEQ(i, -12)
	i = - + t2 * t4; arktest.assertEQ(i, -8)
	i = - ~ t2 * t4; arktest.assertEQ(i, 12)
	i = ~ + t2 * t4; arktest.assertEQ(i, -12)
	i = ~ - t2 * t4; arktest.assertEQ(i, 4)

	// division
	i = + t8 / t2; arktest.assertEQ(i, 4)
	i = + + t8 / t2; arktest.assertEQ(i, 4)
	i = - t8 / t2; arktest.assertEQ(i, -4)
	i = - - t8 / t2; arktest.assertEQ(i, 4)
	i = ~ t8 / t2; arktest.assertEQ(i, -4)
	i = ~ ~ t8 / t2; arktest.assertEQ(i, 4)
	i = + - t8 / t2; arktest.assertEQ(i, -4)
	i = + ~ t8 / t2; arktest.assertEQ(i, -4)
	i = - + t8 / t2; arktest.assertEQ(i, -4)
	i = - ~ t8 / t2; arktest.assertEQ(i, 4)
	i = ~ + t8 / t2; arktest.assertEQ(i, -4)
	i = ~ - t8 / t2; arktest.assertEQ(i, 3)

	// modulo
	i = + t8 % t3; arktest.assertEQ(i, 2)
	i = + + t8 % t3; arktest.assertEQ(i, 2)
	i = - t8 % t3; arktest.assertEQ(i, -2)
	i = - - t8 % t3; arktest.assertEQ(i, 2)
	i = ~ t8 % t2; arktest.assertEQ(i, -1)
	i = ~ ~ t8 % t3; arktest.assertEQ(i, 2)
	i = + - t8 % t3; arktest.assertEQ(i, -2)
	i = + ~ t8 % t2; arktest.assertEQ(i, -1)
	i = - + t8 % t3; arktest.assertEQ(i, -2)
	i = - ~ t8 % t2; arktest.assertEQ(i, 1)
	i = ~ + t8 % t2; arktest.assertEQ(i, -1)
	i = ~ - t8 % t2; arktest.assertEQ(i, 1)

	// addition
	i = + t2 + t4; arktest.assertEQ(i, 6)
	i = + + t2 + t4; arktest.assertEQ(i, 6)
	i = - t2 + t4; arktest.assertEQ(i, 2)
	i = - - t2 + t4; arktest.assertEQ(i, 6)
	i = ~ t2 + t4; arktest.assertEQ(i, 1)
	i = ~ ~ t2 + t4; arktest.assertEQ(i, 6)
	i = + - t2 + t4; arktest.assertEQ(i, 2)
	i = + ~ t2 + t4; arktest.assertEQ(i, 1)
	i = - + t2 + t4; arktest.assertEQ(i, 2)
	i = - ~ t2 + t4; arktest.assertEQ(i, 7)
	i = ~ + t2 + t4; arktest.assertEQ(i, 1)
	i = ~ - t2 + t4; arktest.assertEQ(i, 5)

	// subtraction
	i = + t2 - t4; arktest.assertEQ(i, -2)
	i = + + t2 - t4; arktest.assertEQ(i, -2)
	i = - t2 - t4; arktest.assertEQ(i, -6)
	i = - - t2 - t4; arktest.assertEQ(i, -2)
	i = ~ t2 - t4; arktest.assertEQ(i, -7)
	i = ~ ~ t2 - t4; arktest.assertEQ(i, -2)
	i = + - t2 - t4; arktest.assertEQ(i, -6)
	i = + ~ t2 - t4; arktest.assertEQ(i, -7)
	i = - + t2 - t4; arktest.assertEQ(i, -6)
	i = - ~ t2 - t4; arktest.assertEQ(i, -1)
	i = ~ + t2 - t4; arktest.assertEQ(i, -7)
	i = ~ - t2 - t4; arktest.assertEQ(i, -3)

	// casting
	i = (+ t258).toByte(); arktest.assertEQ(i, 2)
	i = (+ + t258).toByte(); arktest.assertEQ(i, 2)
	i = (- t258).toByte(); arktest.assertEQ(i, -2)
	i = (- - t258).toByte(); arktest.assertEQ(i, 2)
	i = (~ t258).toByte(); arktest.assertEQ(i, -3)
	i = (~ ~ t258).toByte(); arktest.assertEQ(i, 2)
	i = (+ - t258).toByte(); arktest.assertEQ(i, -2)
	i = (+ ~ t258).toByte(); arktest.assertEQ(i, -3)
	i = (- + t258).toByte(); arktest.assertEQ(i, -2)
	i = (- ~ t258).toByte(); arktest.assertEQ(i, 3)
	i = (~ + t258).toByte(); arktest.assertEQ(i, -3)
	i = (~ - t258).toByte(); arktest.assertEQ(i, 1)

	// right shift
	i = + t255 >> t3; arktest.assertEQ(i, 31)
	i = + + t255 >> t3; arktest.assertEQ(i, 31)
	i = - t255 >> t3; arktest.assertEQ(i, -32)
	i = - - t255 >> t3; arktest.assertEQ(i, 31)
	i = ~ t255 >> t3; arktest.assertEQ(i, -32)
	i = ~ ~ t255 >> t3; arktest.assertEQ(i, 31)
	i = + - t255 >> t3; arktest.assertEQ(i, -32)
	i = + ~ t255 >> t3; arktest.assertEQ(i, -32)
	i = - + t255 >> t3; arktest.assertEQ(i, -32)
	i = - ~ t255 >> t3; arktest.assertEQ(i, 32)
	i = ~ + t255 >> t3; arktest.assertEQ(i, -32)
	i = ~ - t255 >> t3; arktest.assertEQ(i, 31)

	// right shift unsigned
	i = + t255 >>> t3; arktest.assertEQ(i, 31)
	i = + + t255 >>> t3; arktest.assertEQ(i, 31)
	i = - t255 >>> t3; arktest.assertEQ(i, 536870880)
	i = - - t255 >>> t3; arktest.assertEQ(i, 31)
	i = ~ t255 >>> t3; arktest.assertEQ(i, 536870880)
	i = ~ ~ t255 >>> t3; arktest.assertEQ(i, 31)
	i = + - t255 >>> t3; arktest.assertEQ(i, 536870880)
	i = + ~ t255 >>> t3; arktest.assertEQ(i, 536870880)
	i = - + t255 >>> t3; arktest.assertEQ(i, 536870880)
	i = - ~ t255 >>> t3; arktest.assertEQ(i, 32)
	i = ~ + t255 >>> t3; arktest.assertEQ(i, 536870880)
	i = ~ - t255 >>> t3; arktest.assertEQ(i, 31)

	// left shift
	i = + t255 << t3; arktest.assertEQ(i, 2040)
	i = + + t255 << t3; arktest.assertEQ(i, 2040)
	i = - t255 << t3; arktest.assertEQ(i, -2040)
	i = - - t255 << t3; arktest.assertEQ(i, 2040)
	i = ~ t255 << t3; arktest.assertEQ(i, -2048)
	i = ~ ~ t255 << t3; arktest.assertEQ(i, 2040)
	i = + - t255 << t3; arktest.assertEQ(i, -2040)
	i = + ~ t255 << t3; arktest.assertEQ(i, -2048)
	i = - + t255 << t3; arktest.assertEQ(i, -2040)
	i = - ~ t255 << t3; arktest.assertEQ(i, 2048)
	i = ~ + t255 << t3; arktest.assertEQ(i, -2048)
	i = ~ - t255 << t3; arktest.assertEQ(i, 2032)

	// equality
	b = + t2 == 2; arktest.assertTrue(b)
	b = + + t2 == 2; arktest.assertTrue(b)
	b = - t2 == -2; arktest.assertTrue(b)
	b = - - t2 == 2; arktest.assertTrue(b)
	b = ~ t2 == -3; arktest.assertTrue(b)
	b = ~ ~ t2 == 2; arktest.assertTrue(b)
	b = + - t2 == -2; arktest.assertTrue(b)
	b = + ~ t2 == -3; arktest.assertTrue(b)
	b = - + t2 == -2; arktest.assertTrue(b)
	b = - ~ t2 == 3; arktest.assertTrue(b)
	b = ~ + t2 == -3; arktest.assertTrue(b)
	b = ~ - t2 == 1; arktest.assertTrue(b)

	// not equality
	b = + t2 != 2; arktest.assertFalse(b)
	b = + + t2 != 2; arktest.assertFalse(b)
	b = - t2 != -2; arktest.assertFalse(b)
	b = - - t2 != 2; arktest.assertFalse(b)
	b = ~ t2 != -3; arktest.assertFalse(b)
	b = ~ ~ t2 != 2; arktest.assertFalse(b)
	b = + - t2 != -2; arktest.assertFalse(b)
	b = + ~ t2 != -3; arktest.assertFalse(b)
	b = - + t2 != -2; arktest.assertFalse(b)
	b = - ~ t2 != 3; arktest.assertFalse(b)
	b = ~ + t2 != -3; arktest.assertFalse(b)
	b = ~ - t2 != 1; arktest.assertFalse(b)

	// bitwise and
	i = + t255 & t4; arktest.assertEQ(i, 4)
	i = + + t255 & t4; arktest.assertEQ(i, 4)
	i = - t2 & t4; arktest.assertEQ(i, 4)
	i = - - t255 & t4; arktest.assertEQ(i, 4)
	i = ~ t2 & t4; arktest.assertEQ(i, 4)
	i = ~ ~ t255 & t4; arktest.assertEQ(i, 4)
	i = + - t2 & t4; arktest.assertEQ(i, 4)
	i = + ~ t2 & t4; arktest.assertEQ(i, 4)
	i = - + t2 & t4; arktest.assertEQ(i, 4)
	i = - ~ t3 & t4; arktest.assertEQ(i, 4)
	i = ~ + t3 & t4; arktest.assertEQ(i, 4)
	i = ~ - t255 & t4; arktest.assertEQ(i, 4)

	// bitwise or
	i = + t2 | t4; arktest.assertEQ(i, 6)
	i = + + t2 | t4; arktest.assertEQ(i, 6)
	i = - t255 | t4; arktest.assertEQ(i, -251)
	i = - - t2 | t4; arktest.assertEQ(i, 6)
	i = ~ t255 | t4; arktest.assertEQ(i, -252)
	i = ~ ~ t2 | t4; arktest.assertEQ(i, 6)
	i = + - t255 | t4; arktest.assertEQ(i, -251)
	i = + ~ t255 | t4; arktest.assertEQ(i, -252)
	i = - + t255 | t4; arktest.assertEQ(i, -251)
	i = - ~ t255 | t4; arktest.assertEQ(i, 260)
	i = ~ + t255 | t4; arktest.assertEQ(i, -252)
	i = ~ - t255 | t4; arktest.assertEQ(i, 254)

	// bitwise exclusive or
	i = + t255 ^ t4; arktest.assertEQ(i, 251)
	i = + + t255 ^ t4; arktest.assertEQ(i, 251)
	i = - t255 ^ t4; arktest.assertEQ(i, -251)
	i = - - t255 ^ t4; arktest.assertEQ(i, 251)
	i = ~ t255 ^ t4; arktest.assertEQ(i, -252)
	i = ~ ~ t255 ^ t4; arktest.assertEQ(i, 251)
	i = + - t255 ^ t3; arktest.assertEQ(i, -254)
	i = + ~ t3 ^ t2; arktest.assertEQ(i, -2)
	i = - + t255 ^ t3; arktest.assertEQ(i, -254)
	i = - ~ t255 ^ t4; arktest.assertEQ(i, 260)
	i = ~ + t3 ^ t2; arktest.assertEQ(i, -2)
	i = ~ - t4 ^ t255; arktest.assertEQ(i, 252)

	// multiplication assignment
	i = t2 *= + t4; arktest.assertEQ(i, 8); arktest.assertEQ(t2, 8); t2 = 2
	i = t2 *= + + t4; arktest.assertEQ(i, 8); arktest.assertEQ(t2, 8); t2 = 2
	i = t2 *= - t4; arktest.assertEQ(i, -8); arktest.assertEQ(t2, -8); t2 = 2
	i = t2 *= - - t4; arktest.assertEQ(i, 8); arktest.assertEQ(t2, 8); t2 = 2
	i = t2 *= ~ t4; arktest.assertEQ(i, -10); arktest.assertEQ(t2, -10); t2 = 2
	i = t2 *= ~ ~ t4; arktest.assertEQ(i, 8); arktest.assertEQ(t2, 8); t2 = 2
	i = t2 *= + - t4; arktest.assertEQ(i, -8); arktest.assertEQ(t2, -8); t2 = 2
	i = t2 *= + ~ t4; arktest.assertEQ(i, -10); arktest.assertEQ(t2, -10); t2 = 2
	i = t2 *= - + t4; arktest.assertEQ(i, -8); arktest.assertEQ(t2, -8); t2 = 2
	i = t2 *= - ~ t4; arktest.assertEQ(i, 10); arktest.assertEQ(t2, 10); t2 = 2
	i = t2 *= ~ + t4; arktest.assertEQ(i, -10); arktest.assertEQ(t2, -10); t2 = 2
	i = t2 *= ~ - t4; arktest.assertEQ(i, 6); arktest.assertEQ(t2, 6); t2 = 2

	// division assignment
	i = t8 /= + t2; arktest.assertEQ(i, 4); arktest.assertEQ(t8, 4); t8 = 8
	i = t8 /= + + t2; arktest.assertEQ(i, 4); arktest.assertEQ(t8, 4); t8 = 8
	i = t8 /= - t2; arktest.assertEQ(i, -4); arktest.assertEQ(t8, -4); t8 = 8
	i = t8 /= - - t2; arktest.assertEQ(i, 4); arktest.assertEQ(t8, 4); t8 = 8
	i = t8 /= ~ t2; arktest.assertEQ(i, -2); arktest.assertEQ(t8, -2); t8 = 8
	i = t8 /= ~ ~ t2; arktest.assertEQ(i, 4); arktest.assertEQ(t8, 4); t8 = 8
	i = t8 /= + - t2; arktest.assertEQ(i, -4); arktest.assertEQ(t8, -4); t8 = 8
	i = t8 /= + ~ t2; arktest.assertEQ(i, -2); arktest.assertEQ(t8, -2); t8 = 8
	i = t8 /= - + t2; arktest.assertEQ(i, -4); arktest.assertEQ(t8, -4); t8 = 8
	i = t8 /= - ~ t2; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 /= ~ + t2; arktest.assertEQ(i, -2); arktest.assertEQ(t8, -2); t8 = 8
	i = t8 /= ~ - t2; arktest.assertEQ(i, 8); arktest.assertEQ(t8, 8); t8 = 8

	// modulo assignment
	i = t8 %= + t3; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= + + t3; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= - t3; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= - - t3; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= ~ t2; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= ~ ~ t3; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= + - t3; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= + ~ t2; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= - + t3; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= - ~ t2; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= ~ + t2; arktest.assertEQ(i, 2); arktest.assertEQ(t8, 2); t8 = 8
	i = t8 %= ~ - t255; arktest.assertEQ(i, 8); arktest.assertEQ(t8, 8); t8 = 8

	// addition assignment
	i = t2 += + t4; arktest.assertEQ(i, 6); t2 = 2
	i = t2 += + + t4; arktest.assertEQ(i, 6); t2 = 2
	i = - t2 + t4; arktest.assertEQ(i, 2); t2 = 2
	i = - - t2 + t4; arktest.assertEQ(i, 6); t2 = 2
	i = ~ t2 + t4; arktest.assertEQ(i, 1); t2 = 2
	i = ~ ~ t2 + t4; arktest.assertEQ(i, 6); t2 = 2
	i = + - t2 + t4; arktest.assertEQ(i, 2); t2 = 2
	i = + ~ t2 + t4; arktest.assertEQ(i, 1); t2 = 2
	i = - + t2 + t4; arktest.assertEQ(i, 2); t2 = 2
	i = - ~ t2 + t4; arktest.assertEQ(i, 7); t2 = 2
	i = ~ + t2 + t4; arktest.assertEQ(i, 1); t2 = 2
	i = ~ - t2 + t4; arktest.assertEQ(i, 5); t2 = 2

	// subtraction assignment
	i = t2 -= + t4; arktest.assertEQ(i, -2); arktest.assertEQ(t2, -2); t2 = 2
	i = t2 -= + + t4; arktest.assertEQ(i, -2); arktest.assertEQ(t2, -2); t2 = 2
	i = t2 -= - t4; arktest.assertEQ(i, 6); arktest.assertEQ(t2, 6); t2 = 2
	i = t2 -= - - t4; arktest.assertEQ(i, -2); arktest.assertEQ(t2, -2); t2 = 2
	i = t2 -= ~ t4; arktest.assertEQ(i, 7); arktest.assertEQ(t2, 7); t2 = 2
	i = t2 -= ~ ~ t4; arktest.assertEQ(i, -2); arktest.assertEQ(t2, -2); t2 = 2
	i = t2 -= + - t4; arktest.assertEQ(i, 6); arktest.assertEQ(t2, 6); t2 = 2
	i = t2 -= + ~ t4; arktest.assertEQ(i, 7); arktest.assertEQ(t2, 7); t2 = 2
	i = t2 -= - + t4; arktest.assertEQ(i, 6); arktest.assertEQ(t2, 6); t2 = 2
	i = t2 -= - ~ t4; arktest.assertEQ(i, -3); arktest.assertEQ(t2, -3); t2 = 2
	i = t2 -= ~ + t4; arktest.assertEQ(i, 7); arktest.assertEQ(t2, 7); t2 = 2
	i = t2 -= ~ - t4; arktest.assertEQ(i, -1); arktest.assertEQ(t2, -1); t2 = 2

	// right shift assignment
	i = t255 >>= + t3; arktest.assertEQ(i, 31); arktest.assertEQ(t255, 31); t255 = 255
	i = t255 >>= + + t3; arktest.assertEQ(i, 31); arktest.assertEQ(t255, 31); t255 = 255
	i = t255 >>= - t255; arktest.assertEQ(i, 127); arktest.assertEQ(t255, 127); t255 = 255
	i = t255 >>= - - t3; arktest.assertEQ(i, 31); arktest.assertEQ(t255, 31); t255 = 255
	i = t255 >>= ~ t255; arktest.assertEQ(i, 255); arktest.assertEQ(t255, 255); t255 = 255
	i = t255 >>= ~ ~ t3; arktest.assertEQ(i, 31); arktest.assertEQ(t255, 31); t255 = 255
	i = t255 >>= + - t255; arktest.assertEQ(i, 127); arktest.assertEQ(t255, 127); t255 = 255
	i = t255 >>= + ~ t255; arktest.assertEQ(i, 255); arktest.assertEQ(t255, 255); t255 = 255
	i = t255 >>= - + t255; arktest.assertEQ(i, 127); arktest.assertEQ(t255, 127); t255 = 255
	i = t255 >>= - ~ t3; arktest.assertEQ(i, 15); arktest.assertEQ(t255, 15); t255 = 255
	i = t255 >>= ~ + t255; arktest.assertEQ(i, 255); arktest.assertEQ(t255, 255); t255 = 255
	i = t255 >>= ~ - t3; arktest.assertEQ(i, 63); arktest.assertEQ(t255, 63); t255 = 255

	// right shift unsigned assignment
	i = t255 >>>= + t3; arktest.assertEQ(i, 31); arktest.assertEQ(t255, 31); t255 = 255
	i = t255 >>>= + + t3; arktest.assertEQ(i, 31); arktest.assertEQ(t255, 31); t255 = 255
	i = t255 >>>= - t255; arktest.assertEQ(i, 127); arktest.assertEQ(t255, 127); t255 = 255
	i = t255 >>>= - - t3; arktest.assertEQ(i, 31); arktest.assertEQ(t255, 31); t255 = 255
	i = t255 >>>= ~ t255; arktest.assertEQ(i, 255); arktest.assertEQ(t255, 255); t255 = 255
	i = t255 >>>= ~ ~ t3; arktest.assertEQ(i, 31); arktest.assertEQ(t255, 31); t255 = 255
	i = t255 >>>= + - t255; arktest.assertEQ(i, 127); arktest.assertEQ(t255, 127); t255 = 255
	i = t255 >>>= + ~ t255; arktest.assertEQ(i, 255); arktest.assertEQ(t255, 255); t255 = 255
	i = t255 >>>= - + t255; arktest.assertEQ(i, 127); arktest.assertEQ(t255, 127); t255 = 255
	i = t255 >>>= - ~ t3; arktest.assertEQ(i, 15); arktest.assertEQ(t255, 15); t255 = 255
	i = t255 >>>= ~ + t255; arktest.assertEQ(i, 255); arktest.assertEQ(t255, 255); t255 = 255
	i = t255 >>>= ~ - t3; arktest.assertEQ(i, 63); arktest.assertEQ(t255, 63); t255 = 255

	// left shift assignment
	i = t255 <<= + t3; arktest.assertEQ(i, 2040); arktest.assertEQ(t255, 2040); t255 = 255
	i = t255 <<= + + t3; arktest.assertEQ(i, 2040); arktest.assertEQ(t255, 2040); t255 = 255
	i = t255 <<= - t3; arktest.assertEQ(i, -536870912); arktest.assertEQ(t255, -536870912); t255 = 255
	i = t255 <<= - - t3; arktest.assertEQ(i, 2040); arktest.assertEQ(t255, 2040); t255 = 255
	i = t255 <<= ~ t3; arktest.assertEQ(i, -268435456); arktest.assertEQ(t255, -268435456); t255 = 255
	i = t255 <<= ~ ~ t3; arktest.assertEQ(i, 2040); arktest.assertEQ(t255, 2040); t255 = 255
	i = t255 <<= + - t3; arktest.assertEQ(i, -536870912); arktest.assertEQ(t255, -536870912); t255 = 255
	i = t255 <<= + ~ t3; arktest.assertEQ(i, -268435456); arktest.assertEQ(t255, -268435456); t255 = 255
	i = t255 <<= - + t3; arktest.assertEQ(i, -536870912); arktest.assertEQ(t255, -536870912); t255 = 255
	i = t255 <<= - ~ t3; arktest.assertEQ(i, 4080); arktest.assertEQ(t255, 4080); t255 = 255
	i = t255 <<= ~ + t3; arktest.assertEQ(i, -268435456); arktest.assertEQ(t255, -268435456); t255 = 255
	i = t255 <<= ~ - t3; arktest.assertEQ(i, 1020); arktest.assertEQ(t255, 1020); t255 = 255

	// bitwise and assignment
	i = t255 &= + t4; arktest.assertEQ(i, 4); arktest.assertEQ(t255, 4); t255 = 255
	i = t255 &= + + t4; arktest.assertEQ(i, 4); arktest.assertEQ(t255, 4); t255 = 255
	i = t4 &= - t2; arktest.assertEQ(i, 4); arktest.assertEQ(t4, 4); t4 = 4
	i = t255 &= - - t4; arktest.assertEQ(i, 4); arktest.assertEQ(t255, 4); t255 = 255
	i = t2 &= ~ t4; arktest.assertEQ(i, 2); arktest.assertEQ(t2, 2); t2 = 2
	i = t255 &= ~ ~ t4; arktest.assertEQ(i, 4); arktest.assertEQ(t255, 4); t255 = 255
	i = t4 &= + - t2; arktest.assertEQ(i, 4); arktest.assertEQ(t4, 4); t4 = 4
	i = t2 &= + ~ t4; arktest.assertEQ(i, 2); arktest.assertEQ(t2, 2); t2 = 2
	i = t4 &= - + t2; arktest.assertEQ(i, 4); arktest.assertEQ(t4, 4); t4 = 4
	i = t3 &= - ~ t4; arktest.assertEQ(i, 1); arktest.assertEQ(t3, 1); t3 = 3
	i = t3 &= ~ + t4; arktest.assertEQ(i, 3); arktest.assertEQ(t3, 3); t3 = 3
	i = t255 &= ~ - t4; arktest.assertEQ(i, 3); arktest.assertEQ(t255, 3); t255 = 255

	// bitwise or assignment
	i = t2 |= + t4; arktest.assertEQ(i, 6); arktest.assertEQ(t2, 6); t2 = 2
	i = t2 |= + + t4; arktest.assertEQ(i, 6); arktest.assertEQ(t2, 6); t2 = 2
	i = t255 |= - t4; arktest.assertEQ(i, -1); arktest.assertEQ(t255, -1); t255 = 255
	i = t2 |= - - t4; arktest.assertEQ(i, 6); arktest.assertEQ(t2, 6); t2 = 2
	i = t4 |= ~ t255; arktest.assertEQ(i, -252); arktest.assertEQ(t4, -252); t4 = 4
	i = t2 |= ~ ~ t4; arktest.assertEQ(i, 6); arktest.assertEQ(t2, 6); t2 = 2
	i = t255 |= + - t4; arktest.assertEQ(i, -1); arktest.assertEQ(t255, -1); t255 = 255
	i = t4 |= + ~ t255; arktest.assertEQ(i, -252); arktest.assertEQ(t4, -252); t4 = 4
	i = t255 |= - + t4; arktest.assertEQ(i, -1); arktest.assertEQ(t255, -1); t255 = 255
	i = t4 |= - ~ t255; arktest.assertEQ(i, 260); arktest.assertEQ(t4, 260); t4 = 4
	i = t4 |= ~ + t255; arktest.assertEQ(i, -252); arktest.assertEQ(t4, -252); t4 = 4
	i = t4 |= ~ - t255; arktest.assertEQ(i, 254); arktest.assertEQ(t4, 254); t4 = 4

	// bitwise exclusive or assignment
	i = t4 ^= + t255; arktest.assertEQ(i, 251); arktest.assertEQ(t4, 251); t4 = 4
	i = t4 ^= + + t255; arktest.assertEQ(i, 251); arktest.assertEQ(t4, 251); t4 = 4
	i = t4 ^= - t255; arktest.assertEQ(i, -251); arktest.assertEQ(t4, -251); t4 = 4
	i = t4 ^= - - t255; arktest.assertEQ(i, 251); arktest.assertEQ(t4, 251); t4 = 4
	i = t4 ^= ~ t255; arktest.assertEQ(i, -252); arktest.assertEQ(t4, -252); t4 = 4
	i = t4 ^= ~ ~ t255; arktest.assertEQ(i, 251); arktest.assertEQ(t4, 251); t4 = 4
	i = t3 ^= + - t255; arktest.assertEQ(i, -254); arktest.assertEQ(t3, -254); t3 = 3
	i = t2 ^= + ~ t3; arktest.assertEQ(i, -2); arktest.assertEQ(t2, -2); t2 = 2
	i = t3 ^= - + t255; arktest.assertEQ(i, -254); arktest.assertEQ(t3, -254); t3 = 3
    i = t4 ^= - ~ t255; arktest.assertEQ(i, 260); arktest.assertEQ(t4, 260); t4 = 4
	i = t2 ^= ~ + t3; arktest.assertEQ(i, -2); arktest.assertEQ(t2, -2); t2 = 2
	i = t255 ^= ~ - t4; arktest.assertEQ(i, 252); arktest.assertEQ(t255, 252); t255 = 255

    
}
