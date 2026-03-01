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
const IS_UNDEFINED = 28;
const IS_DEFINED = 496;

function foo(a?: int ) : int {
    if ( a == undefined) {
        return IS_UNDEFINED;
    } else {
        return IS_DEFINED;
    }
}


function test1() : int {
    let result = foo();
    if (result == IS_UNDEFINED) {
        return 0;
    } else {
        return 1;
    }
}

function test2() : int {
    let result = foo(num);
    if (result == IS_DEFINED) {
        return 0;
    } else {
        return 1
    }
}


function main() : int {
    let counter = 0;

    counter += test1();
    counter += test2();

    if (counter !=  0) {
        return 1;
    }

    return 0;

}
