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

const stsVm = globalThis.gtest.etsVm;
const myRecord = stsVm.getClass('Lrecord/ETSGLOBAL;').myRecord;
const testNewRecordValueFromDynamic = stsVm.getFunction('Lrecord/ETSGLOBAL;', 'testNewRecordValueFromDynamic');

function testGetRecordValue(): void {
  let dayValue = myRecord.day;
  let monthValue = myRecord.month;
  let yearValue = myRecord.year;
  ASSERT_TRUE(dayValue === 199999999);
  ASSERT_TRUE(monthValue === 'two');
  ASSERT_TRUE(yearValue === 3);
}

function testChangeRecordValue(): void {
  myRecord.day = 'one';
  myRecord.month = 2;
  myRecord.year = 'three';
  let dayValue = myRecord.day;
  let monthValue = myRecord.month;
  let yearValue = myRecord.year;
  ASSERT_TRUE(dayValue === 'one');
  ASSERT_TRUE(monthValue === 2);
  ASSERT_TRUE(yearValue === 'three');
}

testGetRecordValue();
testChangeRecordValue();
testNewRecordValueFromDynamic();
