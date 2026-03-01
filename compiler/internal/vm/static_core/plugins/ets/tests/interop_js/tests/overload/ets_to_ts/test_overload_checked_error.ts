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

function testExtendsOverloadConflict(): void {
    try {
        let CSSLogicRule = etsVm.getClass('Ltest_overload_checked_error/CSSLogicRule;');
    } catch (e) {
        ASSERT_TRUE(e.message === 'Multiple override Ltest_overload_checked_error/CSSLogicRule;mergeRule Ltest_overload_checked_error/CSSLogicRule;mergeRule')
    }
}

function testStaInsOverload(): void {
    try {
        let cssLogicRuleIns = etsVm.getFunction('Ltest_overload_checked_error/ETSGLOBAL;','createCSSLogicRule');
        cssLogicRuleIns();
        ASSERT_TRUE(false);
    } catch(e) {
        ASSERT_TRUE(e.message === 'Multiple override Ltest_overload_checked_error/CSSLogicRule;mergeRule Ltest_overload_checked_error/CSSLogicRule;mergeRule')
    }
}


function testCountOverload(): void {
    let CountOverloadRule = etsVm.getClass('Ltest_overload_checked_error/CountOverloadRule;');
    let r = new CountOverloadRule();
    ASSERT_TRUE(r.mergeRule(123));
}

function testTypeOverload(): void {
    try {
        let TypeOverloadRule = etsVm.getClass('Ltest_overload_checked_error/TypeOverloadRule;');
        let r = new TypeOverloadRule();
        r.mergeRule(123)
    } catch (e) {
        ASSERT_TRUE(e.message === 'method has unsupported overloads');
    }
}

function testAll(): void {
    testExtendsOverloadConflict();
    testCountOverload();
    testTypeOverload();
    testStaInsOverload();
}

function main() {
    testAll();
}

main();