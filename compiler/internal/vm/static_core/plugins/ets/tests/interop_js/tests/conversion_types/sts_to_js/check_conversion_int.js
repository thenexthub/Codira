/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const {
    DeferentIntTypes,
} = require('conversion_types.test.abc');

const deferentTypes = new DeferentIntTypes();

function checkIntToNumber() {
    ASSERT_TRUE(typeof deferentTypes.num === 'number');
}

function checkNumberToNumber() {
    ASSERT_TRUE(typeof deferentTypes.numb === 'number');
}

function checkBigInt() {
    ASSERT_TRUE(typeof deferentTypes.bigInt === 'bigint');
}

function checkBigIntToNumber() {
    ASSERT_TRUE(typeof Number(deferentTypes.bigInt) === 'number');
}

function checkFloatToNumber() {
    ASSERT_TRUE(typeof deferentTypes.fl === 'number');
}

function checkIntStringToNumber() {
    ASSERT_TRUE(typeof Number(deferentTypes.intString) === 'number');
}

function checkLongToNumber() {
    ASSERT_TRUE(typeof deferentTypes.l === 'number');
}

function checkDoubleToNumber() {
    ASSERT_TRUE(typeof deferentTypes.doub === 'number');
}

function checkCharToNumber() {
    ASSERT_TRUE(typeof deferentTypes.ch === 'string');
}

function checkByteToNumber() {
    ASSERT_TRUE(typeof deferentTypes.by === 'number');
}

checkIntToNumber();
checkNumberToNumber();
checkBigInt();
checkBigIntToNumber();
checkFloatToNumber();
checkIntStringToNumber();
checkLongToNumber();
checkDoubleToNumber();
checkCharToNumber();
checkByteToNumber();
