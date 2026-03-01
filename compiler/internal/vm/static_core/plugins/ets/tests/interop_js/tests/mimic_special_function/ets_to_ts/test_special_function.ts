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

let GetNumArr = etsVm.getFunction('Lspecial_function/ETSGLOBAL;', 'GetNumArr');
let GetStrArr = etsVm.getFunction('Lspecial_function/ETSGLOBAL;', 'GetStrArr');
let GetBooArr = etsVm.getFunction('Lspecial_function/ETSGLOBAL;', 'GetBooArr');
const numArr = etsVm.getClass('Lspecial_function/ETSGLOBAL;').numArr;
const strArr = etsVm.getClass('Lspecial_function/ETSGLOBAL;').strArr;
const booArr = etsVm.getClass('Lspecial_function/ETSGLOBAL;').booArr;

const INSERT_NUMBER1: number = 9999;
const INSERT_NUMBER2: number = 888;
const INSERT_NUMBER3: number = 77;

const INSERT_STRING1: string = '1111';
const INSERT_STRING2: string = '2222';
const INSERT_STRING3: string = '3333';

const INSERT_BOOLEAN1: boolean = true;
const INSERT_BOOLEAN2: boolean = false;

function testCheckNumberArray(): void {
    ASSERT_TRUE(numArr[0] === 1 && numArr[1] === 3);
    numArr[2] = INSERT_NUMBER1;
    numArr.push(INSERT_NUMBER2);
    numArr.push(INSERT_NUMBER3);
    ASSERT_TRUE(numArr[2] === GetNumArr(2) && numArr[3] === GetNumArr(3) && numArr[4] === GetNumArr(4));
    ASSERT_TRUE(numArr instanceof Array);
}

function testChecStringArray(): void {
    ASSERT_TRUE(strArr[0] === '3' && strArr[2] === '1');
    strArr[2] = INSERT_STRING1;
    strArr.push(INSERT_STRING2);
    strArr.push(INSERT_STRING3);
    ASSERT_TRUE(strArr[2] === GetStrArr(2) && strArr[3] === GetStrArr(3) && strArr[4] === GetStrArr(4));
    ASSERT_TRUE(strArr instanceof Array);
}

function testCheckBooleanArray(): void {
    ASSERT_TRUE(booArr[0] === true && booArr[1] === false);
    booArr[1] = INSERT_BOOLEAN1;
    booArr.push(INSERT_BOOLEAN2);
    ASSERT_TRUE(booArr[1] === GetBooArr(1) && booArr[2] === GetBooArr(2));
    ASSERT_TRUE(booArr instanceof Array);
}

testCheckNumberArray();
testChecStringArray();
testCheckBooleanArray();