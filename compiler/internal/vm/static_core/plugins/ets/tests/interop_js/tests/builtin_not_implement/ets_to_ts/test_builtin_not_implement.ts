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
let getWeakMap = etsVm.getFunction('Lbuiltin_not_implement/ETSGLOBAL;', 'getWeakMap');
let Foo = etsVm.getClass('Lbuiltin_not_implement/Foo;');

const errorString = 'Error: Seamless conversion for class Lstd/containers/WeakMap; is not supported';

function testThorw(func) {
    let res = false;
    try {
        func();
    } catch (err) {
        res = err.toString() === errorString;
    }
    ASSERT_TRUE(res)
}

function testGetWeakMap() {
    testThorw(getWeakMap);
}

function testGetterSetter() {
    let foo = new Foo();
    testThorw(()=> {
        let val = foo.val;
    });
    testThorw(()=> {
        foo.val = new WeakMap<string, string>();
    });
    testThorw(()=> {
        let val = foo.mapVal;
    });
    testThorw(()=> {
        foo.mapVal = new WeakMap<string, string>();
    });
}

testGetWeakMap();
testGetterSetter();
