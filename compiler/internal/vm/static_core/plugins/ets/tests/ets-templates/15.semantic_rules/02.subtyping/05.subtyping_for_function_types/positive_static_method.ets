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

class C {
    //static method reference
    static fooBB(p: Base): Base {return p};
    static fooBD(p: Base): Derived {return new Derived()};
    static fooDB(p: Derived): Base { return new Base()};
    static fooDD(p: Derived): Derived { return p};

    //method with extra param
    static fooBBN(p: Base, n: number): Base {return p};
    static fooBDN(p: Base, n: number): Derived {return new Derived()};
    static fooDBN(p: Derived, n: number): Base { return new Base()};
    static fooDDN(p: Derived, n: number): Derived { return p};

    //method with extra optional param
    static fooBBO(p: Base, n?: number): Base {return p};
    static fooBDO(p: Base, n?: number): Derived {return new Derived()};
    static fooDBO(p: Derived, n?: number): Base { return new Base()};
    static fooDDO(p: Derived, n?: number): Derived { return p};

    //method with extra rest param
    static fooBBR(p: Base, ...n: number[]): Base {return p};
    static fooBDR(p: Base, ...n: number[]): Derived {return new Derived()};
    static fooDBR(p: Derived, ...n: number[]): Base { return new Base()};
    static fooDDR(p: Derived, ...n: number[]): Derived { return p};
}

let foobb = C.fooBB
let foobd = C.fooBD
let foodb = C.fooDB
let foodd = C.fooDD

let foobbn = C.fooBBN
let foobdn = C.fooBDN
let foodbn = C.fooDBN
let fooddn = C.fooDDN

let foobbo = C.fooBBO
let foobdo = C.fooBDO
let foodbo = C.fooDBO
let fooddo = C.fooDDO

let foobbr = C.fooBBR
let foobdr = C.fooBDR
let foodbr = C.fooDBR
let fooddr = C.fooDDR

{{c.decl}}

function main() {
    {{c.check}}
}

{%- endfor %}
