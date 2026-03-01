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

const etsConcatArray1 = etsVm.getClass('Lescompat_interface/ETSGLOBAL;').etsConcatArray1;
const etsConcatArray2 = etsVm.getClass('Lescompat_interface/ETSGLOBAL;').etsConcatArray2;
const arrayConcat = etsVm.getClass('Lescompat_interface/ETSGLOBAL;').arrayConcat;
const error = new Error();

const tsConcatArray1 = [1, 2, 3, 4];
const tsConcatArray2 = [5, 6, 7, 8];

function testEsCompatIfaceInEts(): void {
  // Invoke method with escompat interface as parameter in ets, but take TS object as argument
  ASSERT_EQ(arrayConcat(tsConcatArray1, tsConcatArray2).toString(), '1,2,3,4,5,6,7,8');
}

function testEsCompatIfaceInTs(): void {
  // Invoke method with escompat interface as parameter in ts
  ASSERT_EQ(etsConcatArray1.concat(etsConcatArray2).toString(), '1,2,3,4,5,6,7,8');
  let error = undefined;
  try {
    // Invoke method with escompat interface as parameter in ts with wrong argument type
    etsConcatArray1.concat(new Error());
  } catch (e) {
    error = e;
  }
  ASSERT_TRUE(error !== undefined);
}

testEsCompatIfaceInEts();
testEsCompatIfaceInTs();