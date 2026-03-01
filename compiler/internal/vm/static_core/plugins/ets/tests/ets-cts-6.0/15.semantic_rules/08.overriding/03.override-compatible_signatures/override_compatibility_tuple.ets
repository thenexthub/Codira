/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}

/*---
desc: >-
    There are two cases of type override-compatibility,
    as types are used as either parameter types,
    or return types. For each case there are six kinds of types

    - Tuple type

    Every type is override-compatible with itself (see Invariance).
tags: [{{c.tags}}]
params: {{c.params}}
---*/

let res: number = 0;

type TPL_OB = [Object, Base]
type TPL_BO = [Base, Object]
type TPL_OD = [Object, Derived]
type TPL_DO = [Derived, Object]
type TPL_BD = [Base, Derived]
type TPL_DB = [Derived, Base]
type TPL_BB = [Base, Base]
type TPL_BBB = [Base, [Base, Base]]
type TPL_BBD = [Base, [Base, Derived]]
type TPL_DD = [Derived, Derived]
type TPL_DDD = [Derived, [Derived, Derived]]
type TPL_OOO = [Object, [Object, Object]]

class Base {
    {{c.base|indent}}
}

class Derived extends Base {
    {{c.derived|indent}}
}

let ob: TPL_OB = [new Object(), new Base()]
let od: TPL_OD = [new Object(), new Derived()]
let bb: TPL_BB = [new Base(), new Base()]
let bd: TPL_BD = [new Base(), new Derived()]
let db: TPL_DB = [new Derived(), new Base()]
let dd: TPL_DD = [new Derived(), new Derived()]
let bbb: TPL_BBB = [new Base(), [new Base(), new Base()]]
let bbd: TPL_BBD = [new Base(), [new Base(), new Derived()]]
let ddd: TPL_DDD = [new Derived(), [new Derived(), new Derived()]]

function main() {
    {{(c.call or 'return')|indent}}
}
{% endfor %}
