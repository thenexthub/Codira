/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}
/*---
desc: >-
    If the index expression value of an array is less than zero, greater than or equal to that array’s length, then RangeError is thrown.
---*/

let arr: number[] = [{{c.list}}];
let index: int = {{c.index}};
let a: number;

function foo(): void {
	let a: number = arr[index];
}

let checkException = (e: Error): boolean => { return e instanceof RangeError && e.message.contains("Out of bounds", 0) }

function main() {
    arktest.expectThrow(() => foo(), checkException)
	arktest.assertEQ(a, 0);
}
{% endfor %}
