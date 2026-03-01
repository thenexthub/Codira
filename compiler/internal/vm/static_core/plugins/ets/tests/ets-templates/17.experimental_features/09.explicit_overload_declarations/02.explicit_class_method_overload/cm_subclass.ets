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
desc: Inheritance of overload declaration.
---*/

class A {
    overload foo { fooN, fooS }
    fooN(n: number): string { return 'A.fooN' }
    fooS(s: string): string { return 'A.fooS' }
}

class B extends A {
    override fooN(n: number): string { return 'B.fooN' }
}

class C extends A {
    fooI(n: int): string { return 'C.fooI' }
    overload foo { {{c.list}} }
}

function main() {
    let a: A = new A();
    let b: B = new B();
    let c: C = new C();
    arktest.assertEQ('{{c.value}}', {{c.class}}.foo({{c.param}}));
}
{%- endfor %}
