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

 /*---
desc: 4.7.4 Optional Parameters Issue 15372 test
tags: []

---*/

const num: int = 6;
const IS_ALL_UNDEFINED = 28;
const IS_ALL_DEFINED = 496;
const IS_PARAM_UNDEFINED = 28 + 1;
const IS_PARAM1_DEFINED = 28 + 2;
const IS_PARAM12_DEFINED = 28 + 3;
const TEST_ERROR = 8128


class X {
    foo (a?: int, b?: int ) : int {
        if (a == undefined && b == undefined) {
            return IS_ALL_UNDEFINED;
        }
        if (a != undefined && b == undefined) {
            return IS_PARAM1_DEFINED
        }
        if (a != undefined && b != undefined) {
            return IS_ALL_DEFINED
        }

        return TEST_ERROR;
    }

    foo2 (a?: int, b?: int, c?: int ) : int {
        if (a == undefined && b == undefined && c == undefined) {
            return IS_ALL_UNDEFINED;
        }
        if (a != undefined && b == undefined && c == undefined ) {
            return IS_PARAM1_DEFINED
        }
        if (a != undefined && b != undefined && c == undefined) {
            return IS_PARAM12_DEFINED
        }
        if (a != undefined && b != undefined && c != undefined) {
            return IS_ALL_DEFINED
        }

        return TEST_ERROR;
    }
}


function test1() : int {
    let x: X = new X();
    let result = x.foo();
    if (result == IS_ALL_UNDEFINED) {
        return 0;
    }

    return 1;
}

function test2() : int {
    let x: X = new X();
    let result = x.foo(num);
    if (result == IS_PARAM1_DEFINED) {
        return 0;
    }

    return 1;
}

function test3() : int {
    let x: X = new X();
    let result = x.foo(num, num);
    if (result == IS_ALL_DEFINED) {
        return 0;
    }

    return 1;
}


function test4() : int {
    let x : X = new X();
    let result = x.foo2();
    if (result == IS_ALL_UNDEFINED) {
        return 0;
    }

    return 1;
}

function test5() : int {
    let x : X = new X();
    let result = x.foo2(num);
    if (result == IS_PARAM1_DEFINED) {
        return 0;
    }

    return 1;
}


function test6() : int {
    let x : X = new X();
    let result = x.foo2(num, num);
    if (result == IS_PARAM12_DEFINED) {
        return 0;
    }


    return 1;
}

function test7() : int {
    let x : X = new X();
    let result = x.foo2(num, num, num);
    if (result == IS_ALL_DEFINED) {
        return 0;
    }

    return 1;
}

function main() : int {
    let counter = 0;

    counter += test1();
    counter += test2();
    counter += test3();
    counter += test4();
    counter += test5();
    counter += test6();
    counter += test7();

    if (counter ==  0) {
        return 0;
    }

    return 1;

}
