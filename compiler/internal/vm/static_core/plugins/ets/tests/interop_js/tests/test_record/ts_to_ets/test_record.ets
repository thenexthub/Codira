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

const module = ESValue.load('record');
const myRecord = module.getProperty('myRecord');
const originalRecord = module.getProperty('originalRecord');
const testNewRecordValueFromStatic = module.getProperty('testNewRecordValueFromStatic');

function testGetRecordValue(): boolean {
  let dayValue = myRecord.getProperty('day');
  let monthValue = myRecord.getProperty('month');
  let yearValue = myRecord.getProperty('year');
  arktest.assertTrue(dayValue.toNumber() === 199999999);
  arktest.assertTrue(monthValue.toString() === 'two');
  arktest.assertTrue(yearValue.toNumber() === 3);
  return true;
}

function testChangeRecordValue(): boolean {
  originalRecord.setProperty('day', ESValue.wrapString('one'));
  originalRecord.setProperty('month', ESValue.wrapString('two'));
  originalRecord.setProperty('year', ESValue.wrapString('three'));

  return testNewRecordValueFromStatic.invoke().toBoolean();
}
