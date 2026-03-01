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

const genericDefaultInt = getFunction('genericDefaultInt');
const genericDefaultString = getFunction('genericDefaultString');
const genericDefaultBool = getFunction('genericDefaultBool');
const genericDefaultArr = getFunction('genericDefaultArr');
const genericDefaultObj = getFunction('genericDefaultObj');
const genericDefaultUnion = getFunction('genericDefaultUnion');
const genericDefaultTuple = getFunction('genericDefaultTuple');
const genericDefaultIntCallFromEts = getFunction('genericDefaultIntCallFromEts');
const genericDefaultStringCallFromEts = getFunction('genericDefaultStringCallFromEts');
const genericDefaultBoolCallFromEts = getFunction('genericDefaultBoolCallFromEts');
const genericDefaultArrCallFromEts = getFunction('genericDefaultBoolCallFromEts');
const genericDefaultObjCallFromEts = getFunction('genericDefaultObjCallFromEts');
const genericDefaultUnionCallFromEts = getFunction('genericDefaultUnionCallFromEts');
const genericDefaultTupleCallFromEts = getFunction('genericDefaultTupleCallFromEts');
const genericDefaultLiteral = getFunction('genericDefaultLiteral');
const genericDefaultLiteralCallFromEts = getFunction('genericDefaultLiteralCallFromEts');

function checkGenericDefaultInt() {
    ASSERT_TRUE(genericDefaultInt(jsInt) === jsInt);
}

function checkGenericDefaultString() {
    ASSERT_TRUE(genericDefaultString(jsString) === jsString);
}

function checkGenericDefaultBool() {
    ASSERT_TRUE(genericDefaultBool(jsBool) === jsBool);
}

function checkGenericDefaultArr() {
    const res = genericDefaultArr(jsArr);
    ASSERT_TRUE(checkArray(res) && res[0] === jsArr[0]);
}

function checkGenericDefaultObj() {
    ASSERT_TRUE(checkObj(genericDefaultObj(jsObj)));
}

function checkGenericDefaultTuple() {
    const res = genericDefaultTuple(jsTuple);
    ASSERT_TRUE(checkObj(res) && res.$0 === jsTuple[0] && res.$1 === jsTuple[1]);
}

function checkGenericDefaultUnion() {
    ASSERT_TRUE(genericDefaultUnion(jsString) === jsString);
}


function checkGenericDefaultIntCallFromEts() {
    ASSERT_TRUE(genericDefaultIntCallFromEts() === jsInt);
}

function checkGenericDefaultStringCallFromEts() {
    ASSERT_TRUE(genericDefaultStringCallFromEts() === jsString);
}

function checkGenericDefaultBoolCallFromEts() {
    ASSERT_TRUE(genericDefaultBoolCallFromEts() === jsBool);
}

function checkGenericDefaultArrCallFromEts() {
    const res = genericDefaultArrCallFromEts();
    ASSERT_TRUE(checkArray(res) && res[0] === jsArr[0]);
}

function checkGenericDefaultObjCallFromEts() {
    ASSERT_TRUE(checkObj(genericDefaultObjCallFromEts()));
}

function checkGenericDefaultTupleCallFromEts() {
    const res = genericDefaultTupleCallFromEts();
    ASSERT_TRUE(checkObj(res) && res.$0 === jsTuple[0] && res.$1 === jsTuple[1]);
}

function checkGenericDefaultUnionCallFromEts() {
    ASSERT_TRUE(genericDefaultUnionCallFromEts() === jsInt);
}

function checkGenericDefaultLiteralCallFromEts() {
    ASSERT_TRUE(genericDefaultLiteralCallFromEts() === jsInt);
}

function checkGenericDefaultLiteral() {
    ASSERT_TRUE(genericDefaultLiteral(jsInt) === jsInt);
}


checkGenericDefaultInt();
checkGenericDefaultString();
checkGenericDefaultBool();
checkGenericDefaultArr();
checkGenericDefaultObj();
checkGenericDefaultTuple();
checkGenericDefaultUnion();
checkGenericDefaultIntCallFromEts();
checkGenericDefaultStringCallFromEts();
checkGenericDefaultBoolCallFromEts();
checkGenericDefaultArrCallFromEts();
checkGenericDefaultObjCallFromEts();
checkGenericDefaultTupleCallFromEts();
checkGenericDefaultUnionCallFromEts();
checkGenericDefaultLiteral();
checkGenericDefaultLiteralCallFromEts();
