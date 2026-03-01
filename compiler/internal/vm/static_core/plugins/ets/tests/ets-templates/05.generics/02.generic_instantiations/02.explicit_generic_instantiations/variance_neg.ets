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

{% for c in cases %}
/*---
desc: >-
    An explicit generic instantiation is a language construct, which provides a list of type arguments
    that specify real types or type parameters to substitute corresponding type parameters of a generic.
params: method
tags: [compile-only, negative]
expected_out: {{c.stdout}}
---*/

class Base {}
class Derived extends Base {}

interface BaseI {}
interface DerivedI extends BaseI {}

function fooBase(d1: Derived, d2: Derived): Base { return d1 }
type fooBaseType = (d1: Derived, d2: Derived) => Base
function fooSubtype(d1: Derived, d2: Base): Derived { return d1 }
type fooSubtypeType = (d1: Derived, d2: Base) => Derived

class Cls<T> {
    a: T
    constructor(a: T) {
        this.a = a
    }
}

function main() {
    {{c.use|indent}}
}
{% endfor %}
