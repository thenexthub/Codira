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

let module = ESValue.load('union');
let dynamicObj = module.getProperty('dynamicObj');
let TestClass = module.getProperty("TestClass");

class NewTestClass {
  prop: number = 1;
}

function testUnion() {
  let result = false;
  try {
    dynamicObj.setProperty('unionProp', ESValue.wrapNumber(123));
    arktest.assertTrue(dynamicObj.getProperty('unionProp').toNumber() === 123);

    dynamicObj.setProperty('unionProp', ESValue.wrapString('123'));
    arktest.assertTrue(dynamicObj.getProperty('unionProp').toString() === '123');

    dynamicObj.setProperty('unionProp', ESValue.wrapBoolean(true));
    arktest.assertTrue(dynamicObj.getProperty('unionProp').toBoolean() === true);

    let testInstance = TestClass.instantiate();
    dynamicObj.setProperty('unionProp', testInstance);
    arktest.assertTrue(dynamicObj.getProperty('unionProp').getProperty('name').toString() === 'aaa');

    dynamicObj.setProperty('unionProp', ESValue.wrap(new NewTestClass()));
    arktest.assertTrue(dynamicObj.getProperty('unionProp').getProperty('prop').toNumber() === 1);

    result = true;
  } catch (error) {
    arktest.assertFalse(result);
  }
  return result;
}

