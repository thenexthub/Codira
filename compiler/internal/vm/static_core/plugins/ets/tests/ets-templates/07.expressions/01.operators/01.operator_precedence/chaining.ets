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
desc: chaining operator with other operators
---*/

interface Foo {
    foo (x: int): int
}

class Bar implements Foo {
    intVal?: Int
    bar?: Bar
    row?: Bar[]
    foo(x: int): int {
        return x
    }
    self(): Bar {
        return this;
    }
}

class More extends Bar {}

function main() {

    	let x: Bar = new Bar();

	// grouping; member access
	arktest.assertEQ(x?.foo(1), 1)
	arktest.assertEQ((new Bar())?.foo(2), 2)
	arktest.assertEQ(x?.self()?.foo(3), 3)
	arktest.assertEQ(x.self()?.foo(4), 4)
	arktest.assertEQ(x?.self().foo(5), 5)
	arktest.assertEQ(x?.intVal, undefined)
	arktest.assertEQ(x?.intVal?.toString(), undefined)
	arktest.assertEQ(x.self().self().bar?.bar?.foo(1), undefined)
	arktest.assertEQ(x
		?.self()
		?.
		self()
		.bar
		?.bar
		?.foo(1), undefined)

	// access - lower precedence
	arktest.assertEQ(x?.row?.[0], undefined)
	arktest.assertEQ(x.row?.[0]?.foo(1), undefined)

	// null coalescing
	arktest.assertEQ((x?.bar??null), null)
	arktest.assertEQ((x?.bar??x?.foo(1)), 1)

	// equality
	arktest.assertTrue((new Bar()?.foo(1)==x?.foo(1)))
	arktest.assertFalse((x?.foo(1)!=new Bar()?.foo(1)))
	arktest.assertTrue((x ?. intVal == new Bar() ?. intVal))

	// cast
	let raised = false;
	try {
		let m = new More();
		arktest.assertFalse((m ?. bar as Bar) instanceof Bar)
	} catch (e) {
		arktest.assertTrue(e instanceof ClassCastError)
		raised = true;
	}
	arktest.assertTrue(raised)

	// typeof
	arktest.assertEQ((typeof x ?. bar), 'undefined')
	arktest.assertEQ((typeof x ?. self()), 'object')

	// instanceof
	arktest.assertTrue((x ?. intVal instanceof undefined))
    
}
