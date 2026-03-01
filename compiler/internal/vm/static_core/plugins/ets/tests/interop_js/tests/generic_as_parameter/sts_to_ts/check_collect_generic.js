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
    checkArray,
    jsString,
    jsInt,
    jsBool,
    getFunction,
} = require('generic_as_parameter.test.abc');

const collectGeneric = getFunction('collectGeneric');

function checkCollectGenericSpreadInt() {
    const res = collectGeneric(...[jsInt]);

    ASSERT_TRUE(checkArray(res) && res[0] === jsInt);
}

function checkCollectGenericSpreadString() {
    const res = collectGeneric(...[jsString]);

    ASSERT_TRUE(checkArray(res) && res[0] === jsString);
}

function checkCollectGenericSpreadBool() {
    const res = collectGeneric(...[jsBool]);

    ASSERT_TRUE(checkArray(res) && res[0] === jsBool);
}

function checkCollectGenericInt() {
    const res = collectGeneric(jsInt, jsInt);

    ASSERT_TRUE(checkArray(res) && res[0] === jsInt && res[1] === jsInt);
}

function checkCollectGenericString() {
    const res = collectGeneric(jsString, jsString);

    ASSERT_TRUE(checkArray(res) && res[0] === jsString && res[0] === jsString);
}

function checkCollectGenericBool() {
    const res = collectGeneric(jsBool, jsBool);

    ASSERT_TRUE(checkArray(res) && res[0] === jsBool && res[0] === jsBool);
}

checkCollectGenericSpreadInt();
checkCollectGenericSpreadString();
checkCollectGenericSpreadBool();
checkCollectGenericInt();
checkCollectGenericString();
checkCollectGenericBool();
