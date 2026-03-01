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
    getClass,
    getFunction,
} = require('generic_call_params.test.abc');

const Vector = getClass('Vector');
const genericExtendsClassCallFromSts = getFunction('genericExtendsClassCallFromSts');
const genericExtendsClass = getFunction('genericExtendsClass');

function checkGenericExtendsClass() {
    const GClass = new Vector(jsInt);

    const res = genericExtendsClass(new Vector(jsInt));

    ASSERT_TRUE(res.get() === jsInt + jsInt);
}

function checkGenericExtendsClassCallFromSts() {
    const res = genericExtendsClassCallFromSts(new Vector(jsInt));

    ASSERT_TRUE(res.get() === jsInt + jsInt);
}

checkGenericExtendsClass();
checkGenericExtendsClassCallFromSts();
