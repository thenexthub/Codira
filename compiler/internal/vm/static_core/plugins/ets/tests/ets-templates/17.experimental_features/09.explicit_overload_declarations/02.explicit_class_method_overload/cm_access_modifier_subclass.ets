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
desc: Overload alias and overloaded methods can have different access modifiers.
---*/

class A {
    {{c.mod1}} f1(n: number): string { return 'f1' }
    {{c.mod2}} f2(s: string): string { return 'f2' }
    {{c.modA}} overload foo { f1, f2 }
}

class B extends A {
    {{c.mod3}} f3(s: boolean): string { return 'f3' }
    {{c.modB}} overload foo { f1, f2, f3 }

    test(): string {
        return this.foo(12.6) + this.foo('test') + this.foo(true);
    }
}

function main() {
    let b: B = new B();
    arktest.assertEQ('f1f2f3', b.test());
}
{%- endfor %}
