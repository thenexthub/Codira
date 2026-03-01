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

{%- for c in cases %}
/*---
desc: >-
    Method overload declaration allows declaring an overload alias as a class member for a set of static or instance methods.
    Special tests.
---*/

class B {
    m1(p: int): string { return 'int' }
    m2(p: int, q: object): string { return 'i+o' }
    m3(p: int, q: object, r?: object): string { return 'i+o+o?' }

    overload meth { m1, m2 }
}

class A extends B {
    m4(p: object): string { return "object" }

    overload meth { m1, m2, m3, m4 }
}

function main() {
    let a: A = new A();
    arktest.assertEQ('{{c.value}}', a.meth({{c.param}}));
}
{%- endfor %}
