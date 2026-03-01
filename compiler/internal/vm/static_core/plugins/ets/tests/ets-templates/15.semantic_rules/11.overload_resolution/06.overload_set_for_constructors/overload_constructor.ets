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
desc: Overload Resolution for Overload Declarations - constructor overload.
tags: []
---*/

final class A {
    public size: number = 0;

    constructor conI(a: int) {
        this.size = a;
    }

    constructor conN(a: number) {
        this.size = 100 + a;
    }

    constructor conS(a: string) {
        this.size = a.length;
    }

    overload constructor { {{c.list}} };
}

function main() {
    let i: int = 1;
    let pi: number = 3.14;
    let a: A = new A({{c.param}});

    arktest.assertEQ({{c.result}}, a.size);
}

{% endfor %}
