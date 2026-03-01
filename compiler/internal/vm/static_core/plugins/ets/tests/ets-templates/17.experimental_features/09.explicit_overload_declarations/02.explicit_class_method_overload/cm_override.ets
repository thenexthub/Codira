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

{%- for c in cases %}
/*---
desc: >-
    If a class has an overload declaration, then the overload declaration can be overridden in a subclass.
    If a subclass has no overload declaration, then the declaration from the superclass in used.
    If a subclass has an overload declaration, then the overload declaration must list all methods overloaded in a superclass.
---*/

class A {
    overload foo { fooN, fooS }
    fooN(n: number): string { return 'A.fooN' }
    fooS(s: string): string { return 'A.fooS' }
}

class B extends A {
    override fooN(n: number): string { return 'B.fooN' }
    fooI(n: int): string { return 'B.fooI' }
    overload {{c.name}} { {{c.list}} }
}

function main() {
    let b: B = new B();
    arktest.assertEQ('{{c.value}}', b.{{c.call}}({{c.param}}));
}
{%- endfor %}
