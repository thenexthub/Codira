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

{%- for c in cases %}

/*---
desc: >-
    Value equality is used for operands of bigint type.
    Two bigints are equal if they have the same value.
---*/

function ne(a: bigint, b: bigint): boolean {
    return a != b && b != a
        && a !== b  && b !== a
        && !(a == b) && !(b == a)
        && !(a === b) && !(b === a)
}

let ba: bigint[] = [
    {%- for v in c['values'] %}
    {{v}}{{ ", " if not loop.last }}
    {%- endfor %}
]

function main(): void {
	for (let i: int = 0; i < ba.length - 1; i++) {
		for (let j: int = i + 1; j < ba.length; j++) {
			arktest.assertTrue(ne(ba[i] - 1n, ba[j]))
		}
	}
}
{%- endfor %}
