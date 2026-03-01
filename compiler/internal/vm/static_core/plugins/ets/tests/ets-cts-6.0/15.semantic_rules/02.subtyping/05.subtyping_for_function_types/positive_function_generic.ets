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

//generic function references
function fooBB<T>(p: T): T {return p}
function fooBD<T, G extends T>(p: T): G {return p as G}
function fooDB<G extends T, T>(p: G): T { return p as T};
function fooDD<G extends T, T>(p: G): G { return p};

//generic function with extra param
function fooBBN<T>(p: T, n: number): T {return p};
function fooBDN<T, G extends T>(p: T, n: number): G {return p as G};
function fooDBN<G extends T, T>(p: G, n: number): T { return p as T};
function fooDDN<G extends T, T>(p: G, n: number): G { return p};

//generic function with extra optional param
function fooBBO<T>(p: T, n?: number): T {return p};
function fooBDO<T, G extends T>(p: T, n?: number): G {return p as G};
function fooDBO<G extends T, T>(p: G, n?: number): T { return p as T};
function fooDDO<G extends T, T>(p: G, n?: number): G { return p};

//generic function with extra rest param
function fooBBR<T>(p: T, ...n: number[]): T {return p};
function fooBDR<T, G extends T>(p: T, ...n: number[]): G {return p as G};
function fooDBR<G extends T, T>(p: G, ...n: number[]): T { return p as T};
function fooDDR<G extends T, T>(p: G, ...n: number[]): G { return p};


let foobb = fooBB<Base>
let foobd = fooBD<Base,Derived>
let foodb = fooDB<Derived,Base>
let foodd = fooDD<Derived,Derived>

let foobbn = fooBBN<Base>
let foobdn = fooBDN<Base,Derived>
let foodbn = fooDBN<Derived,Base>
let fooddn = fooDDN<Derived,Derived>

let foobbo = fooBBO<Base>
let foobdo = fooBDO<Base,Derived>
let foodbo = fooDBO<Derived,Base>
let fooddo = fooDDO<Derived,Derived>

let foobbr = fooBBR<Base>
let foobdr = fooBDR<Base,Derived>
let foodbr = fooDBR<Derived,Base>
let fooddr = fooDDR<Derived,Derived>

{{c.decl}}

function main() {
    {{c.check}}
}

{%- endfor %}
