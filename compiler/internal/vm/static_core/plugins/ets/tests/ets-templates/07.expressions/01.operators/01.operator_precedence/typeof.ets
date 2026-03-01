/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
desc: typeof operator with other operators
---*/

function main() {

	let t255: int = 255
	let num: number = 1.0
	let str: string = ""
	let bool: boolean = true
	let obj: Object = new Object()

	// postfix
	str = typeof num++
	arktest.assertEQ(str, "number")
	arktest.assertEQ(num, 2.0)
	str = ""

	str = typeof num--
	arktest.assertEQ(str, "number")
	arktest.assertEQ(num, 1.0)
	str = ""

	str = typeof typeof num++
	arktest.assertEQ(str, "string")
	arktest.assertEQ(num, 2.0)
	str = ""

	str = typeof typeof num--
	arktest.assertEQ(str, "string")
	arktest.assertEQ(num, 1.0)
	str = ""

	// prefix
	str = typeof ++num
	arktest.assertEQ(str, "number")
	arktest.assertEQ(num, 2.0)
	str = ""

	str = typeof --num
	arktest.assertEQ(str, "number")
	arktest.assertEQ(num, 1.0)
	str = ""

	str = typeof typeof ++num
	arktest.assertEQ(str, "string")
	arktest.assertEQ(num, 2.0)
	str = ""

	str = typeof typeof --num
	arktest.assertEQ(str, "string")
	arktest.assertEQ(num, 1.0)
	str = ""

	// unary plus
	str = typeof +num
	arktest.assertEQ(str, "number")
	arktest.assertEQ(num, 1.0)
	str = ""

	str = typeof typeof +num
	arktest.assertEQ(str, "string")
	arktest.assertEQ(num, 1.0)
	str = ""

	// unary minus
	str = typeof -num
	arktest.assertEQ(str, "number")
	arktest.assertEQ(num, 1.0)
	str = ""

	str = typeof typeof -num
	arktest.assertEQ(str, "string")
	arktest.assertEQ(num, 1.0)
	str = ""

	// not
	str = typeof !bool
	arktest.assertEQ(str, "boolean")
	arktest.assertTrue(bool)
	str = ""

	str = typeof typeof !bool
	arktest.assertEQ(str, "string")
	arktest.assertTrue(bool)
	str = ""

	// xor
	str = typeof ~t255
	arktest.assertEQ(str, "number")
	str = ""

	str = typeof typeof ~t255
	arktest.assertEQ(str, "string")
	str = ""

	// addition
	num = 42.0
	str = typeof num + num
	arktest.assertEQ(str, "number42")
	str = ""

	num = 42.0
	str = typeof typeof num + num
	arktest.assertEQ(str, "string42")
	str = ""

	num = 42.0
	str = typeof num + typeof num
	arktest.assertEQ(str, "numbernumber")
	str = ""

	num = 42.0
	str = typeof num + typeof typeof num
	arktest.assertEQ(str, "numberstring")
	str = ""

	// cast
	obj = typeof num as Object
	arktest.assertEQ(obj as string, "number")

	obj = typeof typeof str as String
	arktest.assertEQ(obj as string, "string")

	obj = typeof typeof str as String as Object
	arktest.assertEQ(obj as string, "string")

	// relational
	arktest.assertTrue(typeof num > typeof str)
	arktest.assertTrue(typeof num >= typeof str)
	arktest.assertFalse(typeof num == typeof str)
	arktest.assertTrue(typeof typeof num != typeof str)
	arktest.assertTrue(typeof typeof num > typeof str)
	arktest.assertTrue(typeof typeof num >= typeof str)
	arktest.assertFalse(typeof typeof num < typeof str)
	arktest.assertFalse(typeof typeof num <= typeof str)
	arktest.assertFalse(typeof typeof num != typeof typeof str)
	arktest.assertFalse(typeof typeof num > typeof typeof str)
	arktest.assertTrue(typeof typeof num >= typeof typeof str)
	arktest.assertFalse(typeof typeof num < typeof typeof str)
	arktest.assertTrue(typeof typeof num <= typeof typeof str)

	// ternary
	str = typeof num < typeof str ? typeof num : typeof str
	arktest.assertEQ(str, "number")
	str = ""

	str = typeof num > typeof str ? typeof num : typeof str
	arktest.assertEQ(str, "string")
	str = ""

	str = typeof str < typeof typeof str ? typeof typeof num : typeof num + 1
	arktest.assertEQ(str, "number1")
	str = ""

	// assignments
	let s = "1"
	str = s += typeof num
	arktest.assertEQ(str, "1number")
	arktest.assertEQ(s, "1number")
	s = "1"
	str = ""

	str = s += typeof typeof num
	arktest.assertEQ(str, "1string")
	arktest.assertEQ(s, "1string")
	s = "1"
	str = ""

	str = s += typeof typeof num + typeof num
	arktest.assertEQ(str, "1stringnumber")
	arktest.assertEQ(s, "1stringnumber")
}
