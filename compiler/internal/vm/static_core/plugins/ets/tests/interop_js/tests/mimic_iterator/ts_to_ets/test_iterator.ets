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

import {
    numArray,
    strArray,
    booArray,
} from 'iterator_ts';

let numArr = numArray as Array<Number>;
let strArr = strArray as Array<String>;
let booArr = booArray as Array<Boolean>;

function testMimicIteratorNumberForOf() {
    let idx: int = 0;
    for (let elem of numArr) {
        if (elem !== numArr[idx++]) {
            return false;
        }
    }
    return true;
}

function testMimicIteratorNumberKeys() {
    let idx: int = 0;
    for (let elem of numArr.keys()) {
        if (elem !== idx++) {
            return false;
        }
    }
    return true;
}

function testMimicIteratorNumberValues() {
    let idx: int = 0;
    for (let elem of numArr.values()) {
        if (elem !== numArr[idx++]) {
            return false;
        }
    }
    return true;
}

function testMimicIteratorString() {
    let idx: int = 0;
    for (let elem of strArr) {
        if (elem !== strArr[idx++]) {
            return false;
        }
    }
    return true;
}

function testMimicIteratorBoolean() {
    let idx: int = 0;
    for (let elem of booArr) {
        if (elem !== booArr[idx++]) {
            return false;
        }
    }
    return true;
}

function testMimicIteratorBooleanEntries() {
    for (let elem of booArr.entries()) {
        if (elem[1] !== booArr[elem[0].toInt()]) {
            return false;
        }
    }
    return true;
}

function testMimicIteratorStringEntries() {
    for (let elem of strArr.entries()) {
        if (elem[1] !== strArr[elem[0].toInt()]) {
            return false;
        }
    }
    return true;
}

function testMimicIteratorNumberEntries() {
    for (let elem of numArr.entries()) {
        if (elem[1] !== numArr[elem[0].toInt()]) {
            return false;
        }
    }
    return true;
}

