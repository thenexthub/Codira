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

const { getTestModule } = require('sealed_function.test.abc');

const stsMod = getTestModule('sealed_function');

const sumDouble = stsMod.getFunction('sum_double');
const foo = stsMod.getFunction('foo');
const arrow_func = stsMod.getFunction('arrow_func');

function checkSealed() {
    ASSERT_TRUE(Object.isSealed(sumDouble));
    ASSERT_TRUE(Object.isSealed(foo));
    ASSERT_TRUE(Object.isSealed(arrow_func));
}

function checkNewProperty() {
    ASSERT_THROWS(TypeError, () => { foo.newProp = 3; });
    ASSERT_THROWS(TypeError, () => { sumDouble.newProp = 3; });
    ASSERT_THROWS(TypeError, () => { arrow_func.newProp = 3; });
}

function checkNewMethodDefineProperty() {
    ASSERT_THROWS(TypeError, () => Object.defineProperty(foo, 'newMethod', {
        value: () => 'defined method',
        writable: true,
        configurable: true
    }));

    ASSERT_THROWS(TypeError, () => Object.defineProperty(sumDouble, 'newMethod', {
        value: () => 'defined method',
        writable: true,
        configurable: true
    }));

    ASSERT_THROWS(TypeError, () => Object.defineProperty(arrow_func, 'newMethod', {
        value: () => 'defined method',
        writable: true,
        configurable: true
    }));
}

checkSealed();
checkNewProperty();
checkNewMethodDefineProperty();
