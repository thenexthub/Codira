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
tags: [no-warmup]
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

//function type
type fooBBtype = (p: Base) => Base;
type fooBDtype = (p: Base) => Derived;
type fooDBtype = (p: Derived) => Base;
type fooDDtype = (p: Derived) => Derived;

//function with extra param type
type fooBBNtype = (p: Base, n: number) => Base;
type fooBDNtype = (p: Base, n: number) => Derived;
type fooDBNtype = (p: Derived, n: number) => Base;
type fooDDNtype = (p: Derived, n: number) => Derived;

//function with extra optional param type
type fooBBOtype = (p: Base, n: number) => Base;
type fooBDOtype = (p: Base, n: number) => Derived;
type fooDBOtype = (p: Derived, n: number) => Base;
type fooDDOtype = (p: Derived, n: number) => Derived;

//function with extra rest param type
type fooBBRtype = (p: Base, ...n: number[]) => Base;
type fooBDRtype = (p: Base, ...n: number[]) => Derived;
type fooDBRtype = (p: Derived, ...n: number[]) => Base;
type fooDDRtype = (p: Derived, ...n: number[]) => Derived;

let fooSubtype: {{c.subtype}}
let fooSupertype: {{c.supertype}}

function main() {

    //call functions before subtype to supertype assignment
    arktest.assertTrue(fooSubtype({{c.subtype_params}}) instanceof Derived)
    {% if not c.skip_return_check %}
    arktest.assertFalse(fooSupertype({{ c.supertype_params }}) instanceof Derived);
    arktest.assertEQ(fooSupertype({{c.supertype_params}}).b, 'b');
    {% endif %}

    fooSupertype = fooSubtype

    //call overwritten fooSupertype with initial params
    arktest.assertTrue(fooSupertype({{c.supertype_params}}) instanceof Derived)
    arktest.assertEQ(fooSupertype({{c.supertype_params}}).b, 'd')

    {% if c.supertype_params2 %}
    //call overwritten fooSupertype with subtype params and the same amount of supertype params
    //if subtype has less params
    let q = fooSupertype.unsafeCall({{c.supertype_params2}})
    arktest.assertTrue(q instanceof Derived)
    arktest.assertEQ((q as Base).b, 'd')
    {% endif %}

    //call overwritten fooSupertype with subtype params
    let r = fooSupertype.unsafeCall({{c.subtype_params}})
    arktest.assertTrue(r instanceof Derived)
    arktest.assertEQ((r as Base).b, 'd')
}
{%- endfor %}
