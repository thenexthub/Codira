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
let optionalTestInEts = etsVm.getClass('Lets_required/ETSGLOBAL;').optionalTestInEts;
let requiredTestInEts = etsVm.getClass('Lets_required/ETSGLOBAL;').requiredTestInEts;
let checkOptionalTypeDataInEts = etsVm.getFunction('Lets_required/ETSGLOBAL;', 'checkOptionalTypeDataInEts');
let checkRequiredTypeDataInEts = etsVm.getFunction('Lets_required/ETSGLOBAL;', 'checkRequiredTypeDataInEts');
let Test = etsVm.getClass('Lets_required/Test;');

type RequiredTypeInTs = Required<Test>;
type OptionalTypeInTs = Test;

let requiredTestInTs: RequiredTypeInTs = new Test();
requiredTestInTs.name = 'required in ts';
requiredTestInTs.data1 = 5;
requiredTestInTs.data2 = 6;
  
let optionalTestInTs: OptionalTypeInTs = new Test();
optionalTestInTs.name = 'optional in ts';
optionalTestInTs.data1 = 7;
optionalTestInTs.data2 = 8;

function checkOptionalTypeDataInTS(data:OptionalType): Array<Number> {
    return new Array<Number>(data.data1, data.data2);
}

function checkRequiredTypeDataInTS(data:RequiredType): Array<Number> {
    return new Array<Number>(data.data1, data.data2);
}

ASSERT_TRUE(checkOptionalTypeDataInEts(optionalTestInEts, 1, 2));
ASSERT_TRUE(checkRequiredTypeDataInEts(optionalTestInEts, 1, 2));
ASSERT_TRUE(checkOptionalTypeDataInEts(requiredTestInEts, 3, 4));
ASSERT_TRUE(checkRequiredTypeDataInEts(requiredTestInEts, 3, 4));

let result = checkOptionalTypeDataInTS(optionalTestInEts);
ASSERT_TRUE(result[0] === 1 && result[1] === 2);
result = checkRequiredTypeDataInTS(optionalTestInEts);
ASSERT_TRUE(result[0] === 1 && result[1] === 2);
result = checkOptionalTypeDataInTS(requiredTestInEts);
ASSERT_TRUE(result[0] === 3 && result[1] === 4);
result = checkRequiredTypeDataInTS(requiredTestInEts);
ASSERT_TRUE(result[0] === 3 && result[1] === 4);

result = checkOptionalTypeDataInTS(requiredTestInTs);
ASSERT_TRUE(result[0] === 5 && result[1] === 6);
result = checkOptionalTypeDataInTS(optionalTestInTs);
ASSERT_TRUE(result[0] === 7 && result[1] === 8);
result = checkRequiredTypeDataInTS(requiredTestInTs);
ASSERT_TRUE(result[0] === 5 && result[1] === 6);
result = checkRequiredTypeDataInTS(optionalTestInTs);
ASSERT_TRUE(result[0] === 7 && result[1] === 8);

ASSERT_TRUE(checkRequiredTypeDataInEts(requiredTestInTs, 5, 6));
ASSERT_TRUE(checkRequiredTypeDataInEts(optionalTestInTs, 7, 8));
ASSERT_TRUE(checkOptionalTypeDataInEts(requiredTestInTs, 5, 6));
ASSERT_TRUE(checkOptionalTypeDataInEts(optionalTestInTs, 7, 8));
