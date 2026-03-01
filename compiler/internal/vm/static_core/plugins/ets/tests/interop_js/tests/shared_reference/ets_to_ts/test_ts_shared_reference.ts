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

const etsVm = globalThis.gtest.etsVm;
const areFuncsEqual1 = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areFuncsEqual1');
const areFuncsEqual2 = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areFuncsEqual2');
const areArraysEqual1 = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areArraysEqual1');
const areArraysEqual2 = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areArraysEqual2');
const areArraysEqual3 = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areArraysEqual3');
const areDatasEqual = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areDatasEqual');
const areMapsEqual = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areMapsEqual');
const areSetsEqual = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areSetsEqual');
const areRangeErrorEqual = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areRangeErrorEqual');
const areSyntaxErrorEqual = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areSyntaxErrorEqual');
const areURIErrorEqual = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'areURIErrorEqual');

export let jsArray = ['foo', 1, true];
export let o = {a:1};
let bar = (): void => {
    print('hello');
}

ASSERT_TRUE(!areFuncsEqual1(bar));
ASSERT_TRUE(areFuncsEqual1(bar));
ASSERT_TRUE(!areFuncsEqual2(bar));
ASSERT_TRUE(areFuncsEqual2(bar));

ASSERT_TRUE(!areArraysEqual1(jsArray));
ASSERT_TRUE(areArraysEqual1(jsArray));
ASSERT_TRUE(areArraysEqual2(jsArray, jsArray));
ASSERT_TRUE(areArraysEqual3(jsArray));

const strTime = '2025-03-01T01:02:03.000Z';
let dateStr: Date = new Date(strTime);
// The same function is called twice to verify that the parameters passed multiple times are consistent. 
// The first call has no cache, while the second call has a cache. Therefore, 
// the first call is expected to return false, and the second call is expected to return successfully.
ASSERT_TRUE(!areDatasEqual(dateStr));
ASSERT_TRUE(areDatasEqual(dateStr));

const map = new Map([['key1', 1]]);
ASSERT_TRUE(!areMapsEqual(map));
ASSERT_TRUE(areMapsEqual(map));

const set = new Set([1, 2, 3]);
ASSERT_TRUE(!areSetsEqual(set));
ASSERT_TRUE(areSetsEqual(set));

const rangeError = new RangeError('Range error example');
ASSERT_TRUE(!areRangeErrorEqual(rangeError));
ASSERT_TRUE(areRangeErrorEqual(rangeError));

const syntaxError = new SyntaxError('Syntax error example');
ASSERT_TRUE(!areSyntaxErrorEqual(syntaxError));
ASSERT_TRUE(areSyntaxErrorEqual(syntaxError));

const uriError = new URIError('URI error example');
ASSERT_TRUE(!areURIErrorEqual(uriError));
ASSERT_TRUE(areURIErrorEqual(uriError));