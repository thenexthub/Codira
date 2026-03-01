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
desc: >-
    If the left-hand argument expression completes abruptly,
    then no part of the right-hand argument expression is evaluated.
---*/

let side_effect: int = 0;

function ok(i: int): int {
    side_effect++;
    return i;
}

function bad(i: int): int {
    const a: int[] = [1, 2];
    side_effect++;
    return a[i];
}

class Dummy {
    x: int;
    y: int;
    z: int;
    constructor(a: int, b: int, c: int) {
        this.x = a;
        this.y = b;
        this.z = c;
    }
    method(a: int, b: int, c: int): int {
        return c * b * a;
    }
}

function test(a: int, b: int, c: int): int {
    return c + b + a;
}

function main(): void {
	side_effect = 0;
	let res: int = 0;

	// 1) normal eval for function args
	res = test(ok(1), ok(1), ok(1));
	arktest.assertEQ(res, 3)
	arktest.assertEQ(side_effect, 3)

	// 2) abrupt eval for function args
	side_effect = res = 0;
	try {
		res = test(ok(1), bad(3), ok(1));
	} catch (e) {
		arktest.assertEQ(side_effect, 2)
		arktest.assertEQ(res, 0)
	}

	// 3) abrupt eval for function args
	side_effect = res = 0;
	try {
		res = test(bad(3), ok(1), bad(3));
	} catch (e) {
		arktest.assertEQ(side_effect, 1)
		arktest.assertEQ(res, 0)
	}

	// 4) abrupt eval for lambda args
	side_effect = res = 0;
	try {
		res = ((a: int, b: int, c: int): int => { return c + b + a; })(
			ok(1), bad(3), ok(1));
	} catch (e) {
		arktest.assertEQ(side_effect, 2)
		arktest.assertEQ(res, 0)
	}

	// 5) abrupt eval for method args
	side_effect = res = 0;
	let d = new Dummy(1, 1, 1);
	try {
		res = d.method(ok(1), bad(3), ok(1));
	} catch (e) {
		arktest.assertEQ(side_effect, 2)
		arktest.assertEQ(res, 0)
	}

	// 6) abrupt eval for ctor args
	side_effect = res = 0;
	try {
		let d = new Dummy(ok(1), bad(3), ok(1));
	} catch (e) {
		arktest.assertEQ(side_effect, 2)
	}
}