/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: exponentiation operator with other operators
---*/

class IntKeepper {
    x: int = 2;
    y: int = 3;
}

function main() {
	let x: number = 0;
	let a: int[] = [2, 3];
	// grouping
	x = (1 + 2)**(2 + 1);
	arktest.assertEQ(x, 27)
	// member access
	let v = new IntKeepper();
	x = v.x ** v.y;
	arktest.assertEQ(x, 8)
	// index
	x = a[1] ** a[0];
	arktest.assertEQ(x, 9)
	// pre/post-fix increment and decrement
	x = 1;
	x = ++x ** ++x;
	arktest.assertEQ(x, 8)
	x = 3;
	x = x-- ** x--;
	arktest.assertEQ(x, 9)
	// unary -/+
	x = -3;
	x = x ** -x;
	arktest.assertEQ(x, -27)
	// ** associativity: right-to-left (4**(3**2))
	x = 4 ** 3 ** 2;
	arktest.assertEQ(x, 262144)
	// multiplication has lower precedence
	x = 2*3**2;
	arktest.assertEQ(x, 18)
	x = 11+3**2-19;
	arktest.assertEQ(x, 1)
    
}

