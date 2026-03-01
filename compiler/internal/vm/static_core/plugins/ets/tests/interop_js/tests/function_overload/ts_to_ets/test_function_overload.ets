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

let module = ESValue.load('function_overload');
let processInputDy = module.getProperty('processInput');
let createArrayDy = module.getProperty('createArray');
let getUserDy = module.getProperty('getUser');

function testProcessInput() {
    let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
    let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
    let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
    let res = (res1.toNumber() === 10) && (res2.toString() === 'HELLO') && (!res3.toBoolean());
    return res;
}

function testCreateArray() {
    // [5]
    let res1 = createArrayDy.invoke(ESValue.wrapNumber(5), ESValue.wrapNumber(1));
    // ['a', 1, 'a', 1]
    let res2 = createArrayDy.invoke(ESValue.wrapString('a'), ESValue.wrapNumber(1), ESValue.wrapNumber(4));
    // [true, true]
    let res3 = createArrayDy.invoke(ESValue.wrapBoolean(true), ESValue.wrapNumber(2));
    let res = (res1.getProperty(0).toNumber() === 5) &&
                (res2.getProperty(0).toString() === 'a') &&
                (res3.getProperty(0).toBoolean() === true);
    return res;
}

function testGetUser() {
    // { id: 123, name: 'John Doe' }
    let res1 = getUserDy.invoke(ESValue.wrapNumber(123));
    // { id: 1, name: 'admin' }
    let res2 = getUserDy.invoke(ESValue.wrapString('admin'));
    let res = res1.getProperty('id').toNumber() === 123 &&
                res2.getProperty('role').toString() === 'admin';
    return res;
}
