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

const { getTestModule } = require('prototype_object.test.abc');
const stsMod = getTestModule('prototype_object');

const sts_obj_class = stsMod.getStdClass('Lstd/core/Object');
const get_user_class = stsMod.getFunction('get_user_class');
const get_base_class = stsMod.getFunction('get_base_class');
const get_super_class = stsMod.getFunction('get_super_class');

const user_obj = get_user_class();
const base_obj = get_base_class();
const super_obj = get_super_class();

const User_class = stsMod.getClass('User_class');
const Base_class = stsMod.getClass('Base_class');
const Super_class = stsMod.getClass('Super_class');

function checkStdCoreObject() {
    ASSERT_TRUE(Object.getPrototypeOf(sts_obj_class) === null);
    ASSERT_TRUE(Object.getPrototypeOf(sts_obj_class.prototype) === null);
}

function checkPrototypeOfCustomObject() {
    ASSERT_TRUE(Object.getPrototypeOf(User_class) === sts_obj_class);
    ASSERT_TRUE((Object.getPrototypeOf(Base_class) === sts_obj_class));
    
    let proto = Object.getPrototypeOf(Super_class);
    ASSERT_TRUE(proto === Base_class);
    ASSERT_TRUE((Object.getPrototypeOf(proto)) === sts_obj_class);
}

function checkToStringCall() {
    ASSERT_TRUE(user_obj.toString !== Object.toString);
    ASSERT_TRUE(base_obj.toString !== Object.toString);
    ASSERT_TRUE(super_obj.toString !== Object.toString);
    user_obj.toString(); 
    base_obj.toString();
    super_obj.toString();
}

function checkCallOfStaticFunctions() {
    ASSERT_TRUE(Object.isSealed(Base_class));
    //As there is no isSealed method in std.core.Object
    ASSERT_THROWS(TypeError, () => Base_class.isSealed(Base_class));
    ASSERT_THROWS(TypeError, () => Super_class.isSealed(Super_class));
}

checkStdCoreObject();
checkPrototypeOfCustomObject();
checkCallOfStaticFunctions();
checkToStringCall();

