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

function testBaseClass() {
    let obj = module.getProperty('Base').instantiate().invokeMethod('getCurrent')
    arktest.assertTrue(obj.typeOf() === 'object')
    arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    return true
}

function testChildClass() {
    let obj = module.getProperty('Child').instantiate().invokeMethod('getSuper')
    arktest.assertTrue(obj.typeOf() === 'object')
    arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    arktest.assertTrue(obj.getProperty('childVal').toNumber() === 2)
    return true
}

function testBaseFunc() {
    let obj = module.getProperty('baseFunc').invoke()
    arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    return true
}

function testChildFunc() {
    let obj = module.getProperty('childFunc').invoke()
    arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    return true
}

function testFooBaseObjInvoke() {
    let fooBaseObj = module.getProperty('fooBaseObj')
    let result = fooBaseObj.getProperty('foo').invoke()
    arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    return true
}

function testFooBaseObjInvokeMethod() {
    let fooBaseObj = module.getProperty('fooBaseObj')
    let result = fooBaseObj.invokeMethod('fooFunc')
    arktest.assertTrue(ESValue.areStrictlyEqual(result, fooBaseObj))
    return true
}

function testFooChildObjInvoke() {
    let fooChildObj = module.getProperty('fooChildObj')
    let result = fooChildObj.getProperty('foo').invoke()
    arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    return true
}

function testFooChildObjInvokeMethod() {
    let fooChildObj = module.getProperty('fooChildObj')
    let result = fooChildObj.invokeMethod('fooFunc')
    arktest.assertTrue(ESValue.areStrictlyEqual(result, fooChildObj))
    return true
}

function testArrowFuncInvoke() {
    let result = module.getProperty('arrowFunc').invoke()
    arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    return true
}

