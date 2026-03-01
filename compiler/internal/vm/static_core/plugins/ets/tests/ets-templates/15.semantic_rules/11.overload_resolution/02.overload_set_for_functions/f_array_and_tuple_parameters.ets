/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Overload Declarations - array and tuple parameters functions overload.
tags: []
---*/

function foo1(a: [int, int]): string {
    return 'foo1';
}

function foo2(a: [number, number]): string {
    return 'foo2';
}

function foo3(a: int[]): string {
    return 'foo3';
}

function foo4(a: number[]): string {
    return 'foo4';
}

overload foo { {{c.list}} };

function main() {
    arktest.assertEQ('{{c.result}}', foo([{{c.call}}]));
}

{% endfor %}
