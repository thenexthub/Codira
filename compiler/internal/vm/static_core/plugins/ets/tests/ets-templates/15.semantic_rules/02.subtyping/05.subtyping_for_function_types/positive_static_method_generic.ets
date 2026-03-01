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
    //static method reference generic
    static fooBB<T>(p: T): T {return p};
    static fooBD<T, G extends T>(p: T): G {return p as G};
    static fooDB<G extends T, T>(p: G): T { return p as T};
    static fooDD<G extends T, T>(p: G): G { return p};

    //static method with extra param
    static fooBBN<T>(p: T, n: number): T {return p};
    static fooBDN<T, G extends T>(p: T, n: number): G {return p as G};
    static fooDBN<G extends T, T>(p: G, n: number): T { return p as T};
    static fooDDN<G extends T, T>(p: G, n: number): G { return p};

    //static method with extra optional param
    static fooBBO<T>(p: T, n?: number): T {return p};
    static fooBDO<T, G extends T>(p: T, n?: number): G {return p as G};
    static fooDBO<G extends T, T>(p: G, n?: number): T { return p as T};
    static fooDDO<G extends T, T>(p: G, n?: number): G { return p};

    //static method with extra rest param
    static fooBBR<T>(p: T, ...n: number[]): T {return p};
    static fooBDR<T, G extends T>(p: T, ...n: number[]): G {return p as G};
    static fooDBR<G extends T, T>(p: G, ...n: number[]): T { return p as T};
    static fooDDR<G extends T, T>(p: G, ...n: number[]): G { return p};
}

let foobb = C.fooBB<Base>
let foobd = C.fooBD<Base,Derived>
let foodb = C.fooDB<Derived,Base>
let foodd = C.fooDD<Derived,Derived>

let foobbn = C.fooBBN<Base>
let foobdn = C.fooBDN<Base,Derived>
let foodbn = C.fooDBN<Derived,Base>
let fooddn = C.fooDDN<Derived,Derived>

let foobbo = C.fooBBO<Base>
let foobdo = C.fooBDO<Base,Derived>
let foodbo = C.fooDBO<Derived,Base>
let fooddo = C.fooDDO<Derived,Derived>

let foobbr = C.fooBBR<Base>
let foobdr = C.fooBDR<Base,Derived>
let foodbr = C.fooDBR<Derived,Base>
let fooddr = C.fooDDR<Derived,Derived>

{{c.decl}}

function main() {
    {{c.check}}
}

{%- endfor %}
