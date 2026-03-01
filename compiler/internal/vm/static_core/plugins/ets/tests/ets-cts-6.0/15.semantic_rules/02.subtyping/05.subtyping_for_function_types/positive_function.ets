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

//function reference
function fooBB(p: Base): Base {return p};
function fooBD(p: Base): Derived {return new Derived()};
function fooDB(p: Derived): Base { return new Base()};
function fooDD(p: Derived): Derived { return p};

//function with extra param
function fooBBN(p: Base, n: number): Base {return p};
function fooBDN(p: Base, n: number): Derived {return new Derived()};
function fooDBN(p: Derived, n: number): Base { return new Base()};
function fooDDN(p: Derived, n: number): Derived { return p};

//function with extra optional param
function fooBBO(p: Base, n?: number): Base {return p};
function fooBDO(p: Base, n?: number): Derived {return new Derived()};
function fooDBO(p: Derived, n?: number): Base { return new Base()};
function fooDDO(p: Derived, n?: number): Derived { return p};

//function with extra rest param
function fooBBR(p: Base, ...n: number[]): Base {return p};
function fooBDR(p: Base, ...n: number[]): Derived {return new Derived()};
function fooDBR(p: Derived, ...n: number[]): Base { return new Base()};
function fooDDR(p: Derived, ...n: number[]): Derived { return p};


let foobb = fooBB
let foobd = fooBD
let foodb = fooDB
let foodd = fooDD

let foobbn = fooBBN
let foobdn = fooBDN
let foodbn = fooDBN
let fooddn = fooDDN

let foobbo = fooBBO
let foobdo = fooBDO
let foodbo = fooDBO
let fooddo = fooDDO

let foobbr = fooBBR
let foobdr = fooBDR
let foodbr = fooDBR
let fooddr = fooDDR

{{c.decl}}

function main() {
    {{c.check|indent}}
}

{%- endfor %}
