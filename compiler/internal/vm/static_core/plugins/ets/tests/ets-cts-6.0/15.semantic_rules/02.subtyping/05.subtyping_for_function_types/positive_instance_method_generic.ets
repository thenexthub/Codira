/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

{%- for c in cases %}

/*---
desc: >-
    Function type F with parameters FP1, ... , FPm and return type
    FR is a subtype of function type S with parameters SP1 , ... , SPn
    and return type SR if all of the following conditions are met:
    • m <= n
    • for each i <= m
        - Parameter type of SPi is a subtype of parameter type of FPi (contravariance), and
        – FPi is a rest parameter if SPi is a rest parameter.
        – FPi is an optional parameter if SPi is an optional parameter.
    • Type FR is a subtype of SR (covariance).
---*/

class Base {}
class Derived extends Base {}

class C {
    //instance method reference generic
    fooBB<T>(p: T): T {return p};
    fooBD<T, G extends T>(p: T): G {return p as G};
    fooDB<G extends T, T>(p: G): T { return p as T};
    fooDD<G extends T, T>(p: G): G { return p};

    //method with extra param
    fooBBN<T>(p: T, n: number): T {return p};
    fooBDN<T, G extends T>(p: T, n: number): G {return p as G};
    fooDBN<G extends T, T>(p: G, n: number): T { return p as T};
    fooDDN<G extends T, T>(p: G, n: number): G { return p};

    //method with extra optional param
    fooBBO<T>(p: T, n?: number): T {return p};
    fooBDO<T, G extends T>(p: T, n?: number): G {return p as G};
    fooDBO<G extends T, T>(p: G, n?: number): T { return p as T};
    fooDDO<G extends T, T>(p: G, n?: number): G { return p};

    //method with extra rest param
    fooBBR<T>(p: T, ...n: number[]): T {return p};
    fooBDR<T, G extends T>(p: T, ...n: number[]): G {return p as G};
    fooDBR<G extends T, T>(p: G, ...n: number[]): T { return p as T};
    fooDDR<G extends T, T>(p: G, ...n: number[]): G { return p};
}

const c: C = new C()

let foobb = c.fooBB<Base>
let foobd = c.fooBD<Base,Derived>
let foodb = c.fooDB<Derived,Base>
let foodd = c.fooDD<Derived,Derived>

let foobbn = c.fooBBN<Base>
let foobdn = c.fooBDN<Base,Derived>
let foodbn = c.fooDBN<Derived,Base>
let fooddn = c.fooDDN<Derived,Derived>

let foobbo = c.fooBBO<Base>
let foobdo = c.fooBDO<Base,Derived>
let foodbo = c.fooDBO<Derived,Base>
let fooddo = c.fooDDO<Derived,Derived>

let foobbr = c.fooBBR<Base>
let foobdr = c.fooBDR<Base,Derived>
let foodbr = c.fooDBR<Derived,Base>
let fooddr = c.fooDDR<Derived,Derived>

{{c.decl}}

function main() {
    {{c.check}}
}

{%- endfor %}
