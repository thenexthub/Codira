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
    A method reference refers to a static or instance method of a class or an interface.
    Type of a method reference is derived from the method signature
---*/

class C {
    static fooVal: number = 13;
    static foo() {
        return C.fooVal;
    }
    barVal: number = 42;
    bar (): number {
        return this.barVal;
    }
}

function main() {

	// Method reference to a static method
	const statRef = C.foo
	arktest.assertEQ(statRef(), 13)

	// Method reference to an instance method
	let c1 = new C;
	let c2 = new C;
	const instRef1 = c1.bar
	const instRef2 = c2.bar
	c2.barVal = 100500;
	arktest.assertEQ(instRef1(), 42)
	arktest.assertEQ(instRef2(), 100500)
    
}