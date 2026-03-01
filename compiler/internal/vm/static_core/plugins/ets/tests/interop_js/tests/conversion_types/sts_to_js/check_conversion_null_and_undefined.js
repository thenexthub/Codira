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
    NullAndUndefinedTypes,
} = require('conversion_types.test.abc');

const deferentTypes = new NullAndUndefinedTypes();

function checkNull() {
    ASSERT_TRUE(typeof deferentTypes.etsNull === 'object');
    ASSERT_TRUE(deferentTypes.etsNull === null);
}

function checkUndefined() {
    ASSERT_TRUE(typeof deferentTypes.etsUndefined === 'undefined');
    ASSERT_TRUE(deferentTypes.etsUndefined === undefined);
}

checkNull();
checkUndefined();
