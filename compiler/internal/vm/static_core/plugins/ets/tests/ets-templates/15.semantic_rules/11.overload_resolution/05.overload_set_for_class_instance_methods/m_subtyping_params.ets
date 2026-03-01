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
desc: Overload Declarations - subtyping parameters methods overload.
tags: []
---*/

class A {}
class B extends A {}

class C {
    foo1(a: A): string {
        return 'foo1';
    }

    foo2(b: B): string {
        return 'foo2';
    }

    overload foo { {{c.list}} };
}

function main() {
    let a: A = new A();
    let b: B = new B();
    let c: C = new C();
    arktest.assertEQ('{{c.result}}', c.foo({{c.call}}));
}

{% endfor %}
