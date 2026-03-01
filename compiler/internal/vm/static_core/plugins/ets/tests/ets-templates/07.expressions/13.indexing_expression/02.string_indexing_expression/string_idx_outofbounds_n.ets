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

{% for c in cases %}
/*---
desc: >-
    If the index expression value of an array is less than zero,
    greater than or equal to that string’s length, then the RangeError is thrown.
---*/

let str: string = "abcdefg"

function foo(): void {
	let val = str[{{c.index}}]
}

let checkException = (e: Error): boolean => { return e instanceof RangeError }

function main() {
    arktest.expectThrow(() => foo(), checkException)
}
{% endfor %}
