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
let readonlyTestInEts = etsVm.getClass('Lets_readonly/ETSGLOBAL;').readonlyTestInEts;
let writableTestInEts = etsVm.getClass('Lets_readonly/ETSGLOBAL;').writableTestInEts;
let checkReadonlyTypeDataInEts = etsVm.getFunction('Lets_readonly/ETSGLOBAL;', 'checkReadonlyTypeDataInEts');
let checkWritableTypeDataInEts = etsVm.getFunction('Lets_readonly/ETSGLOBAL;', 'checkWritableTypeDataInEts');
let Test = etsVm.getClass('Lets_readonly/Test;');
  
type ReadonlyTypeTest_TS = Readonly<Test>;
type WritableTypeTest_TS = Test;

let readonlyTestInTs: ReadonlyTypeTest_TS = new Test('readonly new in ts', 5, 6);
let writableTestInTs: WritableTypeTest_TS = new Test('writable new in ts', 7, 8);

function checkReadonlyTypeDataInTs(data: ReadonlyTypeTest_TS): Array<Number> {
    let val = new Array<Number>(data.data1, data.data2);
    return val;
}

function checkWritableTypeDataInTs(data: WritableTypeTest_TS): Array<Number> {
    let val = new Array<Number>(data.data1, data.data2);
    return val;
}

let result = checkReadonlyTypeDataInTs(readonlyTestInEts);
ASSERT_TRUE(result[0] === 1 && result[1] === 2);
result = checkReadonlyTypeDataInTs(writableTestInEts);
ASSERT_TRUE(result[0] === 3 && result[1] === 4);
result = checkWritableTypeDataInTs(readonlyTestInEts);
ASSERT_TRUE(result[0] === 1 && result[1] === 2);
result = checkWritableTypeDataInTs(writableTestInEts);
ASSERT_TRUE(result[0] === 3 && result[1] === 4);

ASSERT_TRUE(checkReadonlyTypeDataInEts(readonlyTestInEts) === 3);
ASSERT_TRUE(checkReadonlyTypeDataInEts(writableTestInEts) === 7);
ASSERT_TRUE(checkWritableTypeDataInEts(readonlyTestInEts) === 3);
ASSERT_TRUE(checkWritableTypeDataInEts(writableTestInEts) === 7);

result = checkReadonlyTypeDataInTs(readonlyTestInTs);
ASSERT_TRUE(result[0] === 5 && result[1] === 6);
result = checkReadonlyTypeDataInTs(writableTestInTs);
ASSERT_TRUE(result[0] === 7 && result[1] === 8);
result = checkWritableTypeDataInTs(readonlyTestInTs);
ASSERT_TRUE(result[0] === 5 && result[1] === 6);
result = checkWritableTypeDataInTs(writableTestInTs);
ASSERT_TRUE(result[0] === 7 && result[1] === 8);

ASSERT_TRUE(checkReadonlyTypeDataInEts(readonlyTestInTs) === 11);
ASSERT_TRUE(checkReadonlyTypeDataInEts(writableTestInTs) === 15);
ASSERT_TRUE(checkWritableTypeDataInEts(readonlyTestInTs) === 11);
ASSERT_TRUE(checkWritableTypeDataInEts(writableTestInTs) === 15);
