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

class Base {
    b: string = 'b'
}
class Derived extends Base {
    b: string = 'd'
}

interface I {
    //instance method reference
    fooBB(p: Base): Base {return p};
    fooBD(p: Base): Derived {return new Derived()};
    fooDB(p: Derived): Base { return new Base()};
    fooDD(p: Derived): Derived { return p};

    //method with extra param
    fooBBN(p: Base, n: number): Base {return p};
    fooBDN(p: Base, n: number): Derived {return new Derived()};
    fooDBN(p: Derived, n: number): Base { return new Base()};
    fooDDN(p: Derived, n: number): Derived { return p};

    //method with extra optional param
    fooBBO(p: Base, n?: number): Base {return p};
    fooBDO(p: Base, n?: number): Derived {return new Derived()};
    fooDBO(p: Derived, n?: number): Base { return new Base()};
    fooDDO(p: Derived, n?: number): Derived { return p};

    //method with extra rest param
    fooBBR(p: Base, ...n: number[]): Base {return p};
    fooBDR(p: Base, ...n: number[]): Derived {return new Derived()};
    fooDBR(p: Derived, ...n: number[]): Base { return new Base()};
    fooDDR(p: Derived, ...n: number[]): Derived { return p};
}

class C implements I {}

const c: I = new C()

let foobb = c.fooBB
let foobd = c.fooBD
let foodb = c.fooDB
let foodd = c.fooDD

let foobbn = c.fooBBN
let foobdn = c.fooBDN
let foodbn = c.fooDBN
let fooddn = c.fooDDN

let foobbo = c.fooBBO
let foobdo = c.fooBDO
let foodbo = c.fooDBO
let fooddo = c.fooDDO

let foobbr = c.fooBBR
let foobdr = c.fooBDR
let foodbr = c.fooDBR
let fooddr = c.fooDDR

{{c.decl}}

function main() {
    {{c.check}}
}

{%- endfor %}
