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
    getFunction,
} = require('generic_call_params.test.abc');

const applyFunctionGenericArray = getFunction('applyFunctionGenericArray');
const applyFunctionGenericArrayCallFromSts = getFunction('applyFunctionGenericArrayCallFromSts');

function checkApplyFunctionGenericArray() {
    const add = (a, b) => a + b;

    const arr = [jsInt, jsInt];
    ASSERT_TRUE(applyFunctionGenericArray(arr, add) === add(arr[0], arr[1]));
}

function checkApplyFunctionGenericArrayCallFromSts() {
    const arr = [jsInt, jsInt];
    ASSERT_TRUE(applyFunctionGenericArrayCallFromSts(arr) === arr[0] + arr[1]);
}

checkApplyFunctionGenericArray();
checkApplyFunctionGenericArrayCallFromSts();
