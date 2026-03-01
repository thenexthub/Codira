/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE- 2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const etsVm = globalThis.gtest.etsVm;
const strTime = '2025-03-01T01:02:03.000Z';
let dateStr: Date = new Date(strTime);

let testSetDateOfsetHours = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfsetHours');
let testSetDateOfsetMinutes = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfsetMinutes');
let testSetDateOfsetSeconds = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfsetSeconds');
let testSetDateOfsetUTCHours = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfsetUTCHours');
let testSetDateOfsetUTCMinutes = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfsetUTCMinutes');
let testSetDateOfsetUTCSeconds = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfsetUTCSeconds');

// Some Date methods are inconsistent in ArkTS1.2 and ArkTS1.1.  #IC4XA0
ASSERT_TRUE(testSetDateOfsetHours(dateStr));
ASSERT_TRUE(testSetDateOfsetMinutes(dateStr));
ASSERT_TRUE(testSetDateOfsetSeconds(dateStr));
ASSERT_TRUE(testSetDateOfsetUTCHours(dateStr));
ASSERT_TRUE(testSetDateOfsetUTCMinutes(dateStr));
ASSERT_TRUE(testSetDateOfsetUTCSeconds(dateStr));
