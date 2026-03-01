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

const etsVm = globalThis.gtest.etsVm;

const jsInt = 1;
const jsString = 'string';
const jsBool = true;
const jsArr = [jsInt];
const jsObj = {};
const jsTuple = [jsInt, jsBool];

const checkArray = (arg) => arg instanceof Array;
const checkObj = (arg) => arg !== null && typeof arg === 'object' && !Array.isArray(arg);

function getClass(name) {
    return etsVm.getClass(`Lgeneric_as_parameter/test/${name};`);
}

function getFunction(name) {
    return etsVm.getFunction('Lgeneric_as_parameter/test/ETSGLOBAL;', name);
}

module.exports = {
    jsInt,
    jsString,
    jsBool,
    jsArr,
    jsObj,
    jsTuple,
    checkArray,
    checkObj,
    getClass,
    getFunction,
};
