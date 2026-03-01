/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
     Constructor is defined in a subclass.
     All base class constructors are available for call in all derived class constructors.
---*/

let res: number = 0;

type TPL = [boolean, float];
type BoolFun = () => boolean;

class Gen<T> {}

class Base<T> {
    constructor() { res = 1;}
    constructor(i: number) { res = 2;}
    constructor(i: string) { res = 3;}
    constructor(i: byte[]) { res = 5;}
    constructor(i: TPL) { res = 6;}
    constructor(i: BoolFun) { res = 7;}
    constructor(i: Number|undefined|null) { res = 8;}
    constructor(i: Gen<T>) { res = 9;}
}

class Derived<T> extends Base<T> {
    {{c.decl}}
}

function main() {
    {{c.use}}
}

{%- endfor %}
