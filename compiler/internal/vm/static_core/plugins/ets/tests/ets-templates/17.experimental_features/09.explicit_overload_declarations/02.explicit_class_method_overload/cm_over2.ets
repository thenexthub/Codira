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
    Aabstract class tests.
---*/

enum Color { Red, Green = 82, Blue }

type T = [number, string];
type R = [int, boolean];

type SN=string|null
type SU=string|undefined
type SNU=string|null|undefined

abstract class B {
    m01(p: int): string { return "int" }
    m02(p: boolean): string { return "boolean" }
    m03(p: double): string { return "double" }
    m04(p: string): string { return "string" }
    m05(p: Color): string { return "Color" }
    m06(p: T): string { return "T" }
    m07(p: R): string { return "R" }
    m08(p: SN): string { return 'SN' }
    m09(p: SU): string { return 'SU' }
    m10(p: SNU): string { return 'SNU' }
    m11(p: int, q: int): string { return "i+i" }
    m12(p: boolean, q?: object): string { return "b+o?" }

    overload meth { m01, m02, m03, m04, m05 }
}

class A extends B {
    m13(p: object): string { return "object" }
    m14(...p: object[]): string { return "objects" }

    overload meth {
        m01, m02, m03, m04, m05, m06, m07, m08, m09, m10,
        m11, m12, m13, m14
    }
}

function main() {
    let a: A = new A();
    let value: {{c.type}} = {{c.value}};
    arktest.assertEQ(a.meth({{c.param}}), '{{c.result}}')
}
{%- endfor %}
