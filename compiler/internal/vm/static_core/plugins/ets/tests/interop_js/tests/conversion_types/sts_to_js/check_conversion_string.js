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
    DeferentStringTypes,
} = require('conversion_types.test.abc');

const deferentTypes = new DeferentStringTypes();

function checkChar() {
    ASSERT_TRUE(typeof deferentTypes.ch === 'string');
    ASSERT_TRUE(deferentTypes.ch === 'a');
}

function checkString() {
    ASSERT_TRUE(typeof deferentTypes.str === 'string');
    ASSERT_TRUE(deferentTypes.str === 'hello');
}

function checkLiteral() {
    ASSERT_TRUE(typeof deferentTypes.lit === 'string');
    ASSERT_TRUE(deferentTypes.lit === 'one');
}

checkChar();
checkString();
checkLiteral();
