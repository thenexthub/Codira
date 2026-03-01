/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Check incorrect overload code. Function overload
tags: [compile-only, negative]
---*/

function foo(): int {
    return 100;
}

function foo(x: int): string {
    return 'int: ' +  x;
}

function foo(x: {{c.type}}): {{c.type}} {
    return x;
}

function main(): void {
    arktest.assertEQ({{c.res}}, foo({{c.callValue}}));
}

{% endfor %}
