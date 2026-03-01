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

let module = ESValue.load('js_code')
let makeEnum = module.getProperty('makeEnum');
let jsEnum = module.getProperty('jsEnum');
let isEq = module.getProperty('isEq');

function Test1(): boolean {
    let a = makeEnum.invoke(['red', 'green', 'blue']);
    if (a.isNull()) {
        return false;
    } else {
        return true;
    }
}

function Test2(): boolean {
    let a = makeEnum.invoke(['red', 'green', 'blue']);
    if (a.getProperty('red').isNull() || a.getProperty('green').isNull() || a.getProperty('blue').isNull()) {
        return false;
    } else {
        return true;
    }
}

function Test3(): boolean {
    let d = jsEnum.invoke();
    if (d.isNull()) {
        return false;
    } else {
        return true;
    }
}


function Test4(): boolean {
    let d = jsEnum.invoke();
    if (d.getProperty('Up').toString() === 'Up' && d.getProperty('Down').toString() === 'Down') {
        return true;
    } else {
        return false;
    }
}

function Test5(): boolean {
    let d = jsEnum.invoke();
    let u = d.getProperty('Up');
    if (isEq.invoke(u, d.getProperty('Up')).toBoolean()) {
        return true;
    } else {
        return false;
    }
}

function Test6(): boolean {
    let d = jsEnum.invoke();
    let u = d.getProperty('Up');
    if (isEq.invoke('Up', u.toString()).toBoolean()) {
        return true;
    } else {
        return false;
    }
}