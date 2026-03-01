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

const GenericExtends = getClass('GenericExtends');
const genericClassExtendsCallFromSts = getFunction('genericClassExtendsCallFromSts');

function checkClassExtendsInt() {
    ASSERT_TRUE(GenericExtends.get(jsInt) === jsInt);
}

function checkClassExtendsString() {
    ASSERT_TRUE(GenericExtends.get(jsString) === jsString);
}

function checkClassExtendsBool() {
    ASSERT_TRUE(GenericExtends.get(jsBool) === jsBool);
}

function checkClassExtendsArr() {
    const res = GenericExtends.get(jsArr);
    ASSERT_TRUE(checkArray(res) && res[0] === jsArr[0]);
}

function checkClassExtendsMethodCallFromJsInt() {
    ASSERT_TRUE(genericClassExtendsCallFromSts(jsInt) === jsInt);
}

function checkClassExtendsMethodCallFromJsString() {
    ASSERT_TRUE(genericClassExtendsCallFromSts(jsString) === jsString);
}

function checkClassExtendsMethodCallFromJsBool() {
    ASSERT_TRUE(genericClassExtendsCallFromSts(jsBool) === jsBool);
}

function checkClassExtendsMethodCallFromJsArr() {
    const res = genericClassExtendsCallFromSts(jsArr);
    ASSERT_TRUE(checkArray(res) && res[0] === jsArr[0]);
}

checkClassExtendsInt();
checkClassExtendsString();
checkClassExtendsBool();
checkClassExtendsArr();
checkClassExtendsMethodCallFromJsInt();
checkClassExtendsMethodCallFromJsString();
checkClassExtendsMethodCallFromJsBool();
checkClassExtendsMethodCallFromJsArr();
