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
    checkObj,
    checkArray,
    jsInt,
    jsString,
    jsBool,
    jsArr,
    jsObj,
    jsTuple,
    getFunction,
} = require('generic_as_parameter.test.abc');

const anyTypeParameter = getFunction('anyTypeParameter');
const anyTypeParameterExplicitCallFromEtsInt = getFunction('anyTypeParameterExplicitCallFromEtsInt');
const anyTypeParameterExplicitCallFromEtsString = getFunction('anyTypeParameterExplicitCallFromEtsString');
const anyTypeParameterExplicitCallFromEtsBool = getFunction('anyTypeParameterExplicitCallFromEtsBool');
const anyTypeParameterExplicitCallFromEtsArr = getFunction('anyTypeParameterExplicitCallFromEtsArr');
const anyTypeParameterExplicitCallFromEtsObj = getFunction('anyTypeParameterExplicitCallFromEtsObj');
const anyTypeParameterExplicitCallFromEtsUnion = getFunction('anyTypeParameterExplicitCallFromEtsUnion');


function checkAnyTypeParameterInt() {
    ASSERT_TRUE(anyTypeParameter(jsInt) === jsInt);
}

function checkAnyTypeParameterString() {
    ASSERT_TRUE(anyTypeParameter(jsString) === jsString);
}

function checkAnyTypeParameterBool() {
    ASSERT_TRUE(anyTypeParameter(jsBool) === jsBool);
}

function checkAnyTypeParameterArr() {
    const res = anyTypeParameter(jsArr);
    ASSERT_TRUE(checkArray(res) && res[0] === jsArr[0]);
}

function checkAnyTypeParameterObj() {
    ASSERT_TRUE(checkObj(anyTypeParameter(jsObj)));
}

function checkAnyTypeParameterTuple() {
    const res = anyTypeParameter(jsTuple);
    ASSERT_TRUE(checkArray(res) && res[0] === jsTuple[0] && res[1] === jsTuple[1]);
}

function checkAnyTypeParameterExplicitCallFromEtsInt() {
    ASSERT_TRUE(anyTypeParameterExplicitCallFromEtsInt() === jsInt);
}

function checkAnyTypeParameterExplicitCallFromEtsString() {
    ASSERT_TRUE(anyTypeParameterExplicitCallFromEtsString() === jsString);
}

function checkAnyTypeParameterExplicitCallFromEtsBool() {
    ASSERT_TRUE(anyTypeParameterExplicitCallFromEtsBool() === jsBool);
}

function checkAnyTypeParameterExplicitCallFromEtsArr() {
    const res = anyTypeParameterExplicitCallFromEtsArr();
    ASSERT_TRUE(checkArray(res) && res[0] === jsArr[0]);
}

function checkAnyTypeParameterExplicitCallFromEtsObj() {
    ASSERT_TRUE(checkObj(anyTypeParameterExplicitCallFromEtsObj()));
}

function checkAnyTypeParameterExplicitCallFromEtsUnion() {
    ASSERT_TRUE(anyTypeParameterExplicitCallFromEtsUnion() === jsInt);
}

checkAnyTypeParameterInt();
checkAnyTypeParameterString();
checkAnyTypeParameterBool();
checkAnyTypeParameterArr();
checkAnyTypeParameterObj();
checkAnyTypeParameterTuple();
checkAnyTypeParameterExplicitCallFromEtsInt();
checkAnyTypeParameterExplicitCallFromEtsString();
checkAnyTypeParameterExplicitCallFromEtsBool();
checkAnyTypeParameterExplicitCallFromEtsArr();
checkAnyTypeParameterExplicitCallFromEtsObj();
checkAnyTypeParameterExplicitCallFromEtsUnion();
