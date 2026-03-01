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
    getClass,
    getFunction,
} = require('generic_static.test.abc');

const UnionStatic = getClass('UnionStatic');
const genericUnionStaticCallFromSts = getFunction('genericUnionStaticCallFromSts');

function checkGenericUnionInt() {
    ASSERT_TRUE(UnionStatic.get(jsInt) === jsInt);
}

function checkGenericUnionString() {
    ASSERT_TRUE(UnionStatic.get(jsString) === jsString);
}

function checkGenericUnionCallFromJsInt() {
    ASSERT_TRUE(genericUnionStaticCallFromSts(jsInt) === jsInt);
}

function checkGenericUnionCallFromJsString() {
    ASSERT_TRUE(genericUnionStaticCallFromSts(jsString) === jsString);
}

checkGenericUnionInt();
checkGenericUnionString();
checkGenericUnionCallFromJsInt();
checkGenericUnionCallFromJsString();
