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
desc: >-
  Expressions may contain assignments, increment operators, decrement operators,
  method calls and function calls; as a result, an evaluation of an expression
  can produce side effects.
---*/

let sg: String = "a";
let ig: int = 42;

function foo(p: int, s: String): int {
    sg += s;
    ig = -1;
    return p - s.length.toInt();
}

class A {
  meth(p: int, s: String): int {
    sg += s;
    ig = 1;
    return p - s.length.toInt();
  }
}

function main() {

	sg = "a"; // reset due to jit mode warmup
	ig = 42;

	// assignment side effect
	let i: int = (sg += "b").length.toInt() + (ig = 43);
	arktest.assertEQ(i, 45)
	arktest.assertEQ(sg, "ab")
	arktest.assertEQ(ig, 43)

	// increment side effect
	let k: int = ig++ + ++ig;
	arktest.assertEQ(k, 88)
	arktest.assertEQ(ig, 45)

	// decrement side effect
	let j: int = ig-- + --ig;
	arktest.assertEQ(j, 88)
	arktest.assertEQ(ig, 43)

	// function call side effect
	let m: int = foo(j, sg);
	arktest.assertEQ(m, 86)
	arktest.assertEQ(ig, -1)
	arktest.assertEQ(sg, "abab")

	// method call side effect
	let n: int = new A().meth(m, sg);
	arktest.assertEQ(n, 82)
	arktest.assertEQ(ig, 1)
	arktest.assertEQ(sg, "abababab")

    
}
