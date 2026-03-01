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
    jsBaseObj,
    getClass,
    getFunction,
} = require('generic_static.test.abc');

const SubsetByValueStatic = getClass('SubsetByValueStatic');
const genericSubsetByValueStaticCallFromSts = getFunction('genericSubsetByValueStaticCallFromSts');

function checkSubsetByValueStatic() {
    const res = SubsetByValueStatic.get(jsBaseObj);

    ASSERT_TRUE(res.a === jsBaseObj.a);
}

function checkGenericSubsetByValueStaticCallFromSts() {
    const res = genericSubsetByValueStaticCallFromSts(jsBaseObj);

    ASSERT_TRUE(res.a === jsBaseObj.a);
}

checkSubsetByValueStatic();
checkGenericSubsetByValueStaticCallFromSts();
