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
    getFunction,
} = require('generic_call_params.test.abc');

const applyFunctionWithConstraints = getFunction('applyFunctionWithConstraints');
const applyFunctionWithConstraintsFromSts = getFunction('applyFunctionWithConstraintsFromSts');

const concat = (a, b) => a + ' ' + b;

function checkApplyFunctionWithConstraints() {

    ASSERT_TRUE(applyFunctionWithConstraints(jsInt, jsString, concat) === concat(jsInt, jsString));
}

function checkApplyFunctionWithConstraintsFromSts() {
    ASSERT_TRUE(applyFunctionWithConstraintsFromSts(jsInt, jsString) === concat(jsInt, jsString));
}

checkApplyFunctionWithConstraints();
checkApplyFunctionWithConstraintsFromSts();
