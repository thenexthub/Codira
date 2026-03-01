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
desc: Overload Declarations - numeric parameters counstructor overload.
tags: []
---*/

class A {
    foo: string = '';

    constructor fB(a: byte) {
        this.foo = 'fB';
    }

    constructor fS(a: short) {
        this.foo = 'fS';
    }

    constructor fI(a: int) {
        this.foo = 'fI';
    }

    constructor fL(a: long) {
        this.foo = 'fL';
    }

    constructor fF(a: float) {
        this.foo = 'fF';
    }

    constructor fN(a: number) {
        this.foo = 'fN';
    }

    overload constructor { {{c.list}} };
}

function main() {
    let b: byte = 1;
    let s: short = 2;
    let i: int = 3;
    let l: long = 4;
    let f: float = 5.1f;
    let n: number = 6.2;
    let a: A = new A({{c.call}});
    arktest.assertEQ('{{c.result}}', a.foo);
}

{% endfor %}
