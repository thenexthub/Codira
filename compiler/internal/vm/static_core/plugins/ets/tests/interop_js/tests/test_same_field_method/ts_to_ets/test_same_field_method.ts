/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
let B = etsVm.getClass('Ltest_same_field_method/B;');
let A = etsVm.getClass('Ltest_same_field_method/A;');

function testAll() {
    ASSERT_TRUE(B.successResult());
    let b = new B();
    ASSERT_TRUE(b.successResult === 'BsuccessResult');

    ASSERT_TRUE(A.successResult());
    ASSERT_TRUE(!A.successResultB());
    let a = new A();
    ASSERT_TRUE(a.successResult === 'AsuccessResult');
}

function main() {
    testAll();
}

main();
