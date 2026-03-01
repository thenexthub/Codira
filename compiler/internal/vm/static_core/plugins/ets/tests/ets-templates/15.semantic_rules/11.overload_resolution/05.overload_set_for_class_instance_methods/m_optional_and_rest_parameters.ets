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
desc: Overload Declarations - optional and rest parameters methods overload.
tags: []
---*/

class A {
    foo1(a: 'a'|'b'): string {
        return 'foo1';
    }

    foo2(a: string): string {
        return 'foo2';
    }

    foo3(a: string, b?: int): string {
        return 'foo3';
    }

    foo4(a: string, ...b: number[]): string {
        return 'foo4';
    }

    overload foo { {{c.list}} };
}

function main() {
    let a: A = new A()
    arktest.assertEQ('{{c.result}}', a.foo({{c.call}}));
}

{% endfor %}
