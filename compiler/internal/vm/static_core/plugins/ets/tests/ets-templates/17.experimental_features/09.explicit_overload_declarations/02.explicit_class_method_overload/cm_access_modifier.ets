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
    {{c.mod1}} one(n: number): string { return 'one' }
    {{c.mod2}} two(s: string): string { return 'two' }
    {{c.modA}} overload foo { one, two }

    publicFoo(x: number|string): string {
        if (x instanceof number) {
            return this.foo(x);
        } else {
            return this.foo(x);
        }
    }
}

function main() {
    let a: A = new A();
    
    arktest.assertEQ('one', a.{{c.call}}(12.6));
    arktest.assertEQ('two', a.{{c.call}}('Hello!'));
}
{%- endfor %}
