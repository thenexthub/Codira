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
const testBasicUnion = etsVm.getFunction('Lunion/test/ETSGLOBAL;', 'testBasicUnion');
const testChildUnion = etsVm.getFunction('Lunion/test/ETSGLOBAL;', 'testChildUnion');

class TestClass {
  prop: number = 1;
}
function testUnionType(): void {
  let numType = typeof testBasicUnion(1);
  let strType = typeof testBasicUnion('1');
  let boolType = typeof testBasicUnion(true);

  ASSERT_EQ(numType, 'number');
  ASSERT_EQ(strType, 'string');
  ASSERT_EQ(boolType, 'boolean');

  try {
    testChildUnion(new TestClass());
  } catch (err) {
    ASSERT_EQ(err.toString(), 'TypeError: Lstd/interop/js/JSValue; is not assignable to {ULunion/test/Child1;Lunion/test/Child2;}');
  }
}

testUnionType();
