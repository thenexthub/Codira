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
    The execution of the object literal completes abruptly if so does the execution of a name-value pair.
---*/

class A {
    name: string|null = null
    age: number|null = null
}

let a: A = { name: "Alice", age: 4 };
function foo(): void {
  let t: number[] = [1, 2, 3, 4, 5]
  let a: A|null = null;
  a = { name: "Bob", age: t[5] } as A
}

let checkException = (e: Error): boolean => { return e instanceof RangeError }

function main() {
    arktest.expectThrow(() => foo(), checkException)
	arktest.assertEQ(a.age, 4);
}
