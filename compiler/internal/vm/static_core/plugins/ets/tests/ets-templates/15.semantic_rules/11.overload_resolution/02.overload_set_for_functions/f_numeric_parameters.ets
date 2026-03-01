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
desc: Overload Declarations - numeric parameters functions overload.
tags: []
---*/

function fB(a: byte): string {
    return 'fB';
}

function fS(a: short): string {
    return 'fS';
}

function fI(a: int): string {
    return 'fI';
}

function fL(a: long): string {
    return 'fL';
}

function fF(a: float): string {
    return 'fF';
}

function fN(a: number): string {
    return 'fN';
}

overload foo { {{c.list}} };

function main() {
    let b: byte = 1;
    let s: short = 2;
    let i: int = 3;
    let l: long = 4;
    let f: float = 5.1f;
    let n: number = 6.2;
    arktest.assertEQ('{{c.result}}', foo({{c.call}}));
}

{% endfor %}
