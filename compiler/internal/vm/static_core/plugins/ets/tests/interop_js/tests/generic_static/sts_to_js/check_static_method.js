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
    jsInt,
    jsString,
    jsBool,
    jsArr,
    checkArray,
    getClass,
    getFunction,
} = require('generic_static.test.abc');

const GenericStatic = getClass('GenericStatic');
const genericStaticMethodCallFromSts = getFunction('genericStaticMethodCallFromSts');

function checkGenericStaticInt() {
    ASSERT_TRUE(GenericStatic.get(jsInt) === jsInt);
}

function checkGenericStaticString() {
    ASSERT_TRUE(GenericStatic.get(jsString) === jsString);
}

function checkGenericStaticBool() {
    ASSERT_TRUE(GenericStatic.get(jsBool) === jsBool);
}

function checkGenericStaticArr() {
    const res = GenericStatic.get(jsArr);
    ASSERT_TRUE(checkArray(res) && res[0] === jsArr[0]);
}

function checkGenericStaticMethodCallFromJsInt() {
    ASSERT_TRUE(genericStaticMethodCallFromSts(jsInt) === jsInt);
}

function checkGenericStaticMethodCallFromJsString() {
    ASSERT_TRUE(genericStaticMethodCallFromSts(jsString) === jsString);
}

function checkGenericStaticMethodCallFromJsBool() {
    ASSERT_TRUE(genericStaticMethodCallFromSts(jsBool) === jsBool);
}

function checkGenericStaticMethodCallFromJsArr() {
    const res = genericStaticMethodCallFromSts(jsArr);
    ASSERT_TRUE(checkArray(res) && res[0] === jsArr[0]);
}

checkGenericStaticInt();
checkGenericStaticString();
checkGenericStaticBool();
checkGenericStaticArr();
checkGenericStaticMethodCallFromJsInt();
checkGenericStaticMethodCallFromJsString();
checkGenericStaticMethodCallFromJsBool();
checkGenericStaticMethodCallFromJsArr();
