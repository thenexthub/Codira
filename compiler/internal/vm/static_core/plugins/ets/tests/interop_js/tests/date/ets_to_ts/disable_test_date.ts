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

const DATASTR = etsVm.getClass('Ldate/test/ETSGLOBAL;').DATASTR;
const TIMESTAMP = etsVm.getClass('Ldate/test/ETSGLOBAL;').TIMESTAMP;

let etsDate = etsVm.getClass('Ldate/test/ETSGLOBAL;').etsDate;
let etsStrDate = etsVm.getClass('Ldate/test/ETSGLOBAL;').strDate;
let etsStampDate = etsVm.getClass('Ldate/test/ETSGLOBAL;').stampDate;

function checkGetofDate(etsDate: Date, jsDate: Date): void {
    // Some Date methods are inconsistent in ArkTS1.2 and ArkTS1.1.  #IC4XA0
    ASSERT_TRUE(etsDate.toString() === jsDate.toString());
    ASSERT_TRUE(etsDate.toTimeString() === jsDate.toTimeString());

    // Methods involving intl parameters do not take Builtin.  #IC4X7S
    ASSERT_TRUE(etsDate.toLocaleTimeString() === jsDate.toLocaleTimeString());
    ASSERT_TRUE(etsDate.toLocaleTimeString('en-US') === jsDate.toLocaleTimeString('en-US'));
    ASSERT_TRUE(etsDate.toLocaleTimeString('zh-CN') === jsDate.toLocaleTimeString('zh-CN'));
    ASSERT_TRUE(etsDate.toLocaleString() === jsDate.toLocaleString());
    ASSERT_TRUE(etsDate.toLocaleString('en-US') === jsDate.toLocaleString('en-US'));
    ASSERT_TRUE(etsDate.toLocaleString('zh-CN') === jsDate.toLocaleString('zh-CN'))
    ASSERT_TRUE(etsDate.toLocaleDateString('en-US') === jsDate.toLocaleDateString('en-US'))
    ASSERT_TRUE(etsDate.toLocaleDateString('zh-CN') === jsDate.toLocaleDateString('zh-CN'))

}

function testGetofDate(): void {
    let jsDate = new Date(2025, 2, 1, 1, 2, 3, 4);
    let jsStrDate = new Date(DATASTR);
    let jsStampDate = new Date(TIMESTAMP);
    checkGetofDate(etsDate, jsDate);
    checkGetofDate(etsStrDate, jsStrDate);
    checkGetofDate(etsStampDate, jsStampDate);
}

testGetofDate();
