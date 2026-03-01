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

const { getTestModule } = require('sealed_object.test.abc');

const stsMod = getTestModule('sealed_object');

const obj = stsMod.getClass('ETSGLOBAL').my_obj;

function checkNewProperty() {
    ASSERT_THROWS(TypeError, () => { obj.newProp = 3; });
}

function checkDeleteProperty() {
    delete obj.value;

    ASSERT_TRUE(obj.value === 2);
}

function checkChangeProperty() {
    let a = 100;
    obj.value = a;
    ASSERT_TRUE(obj.value === 100);
}

function checkNewMethod() {
    ASSERT_THROWS(TypeError, () => { obj.newMethod = () => 'new method'; });
}

function checkDeleteMethod() {
    delete obj.return_answer;
    
    ASSERT_TRUE(obj.return_answer() === 42);
}

function checkNewMethodDefineProperty() {
    ASSERT_THROWS(TypeError, () => Object.defineProperty(obj, 'newMethod', {
        value: () => 'defined method',
        writable: true,
        configurable: true
    }));
}

function checkNewMethodAssign() {
    ASSERT_THROWS(TypeError, () => Object.assign(obj, { newMethod: () => 'assigned method' }));
}

function checkNewGetterSetter() {
    ASSERT_THROWS(TypeError, () => Object.defineProperty(obj, 'newAccessor', {
        get() { return 'value'; },
        set(v) {},
        configurable: true
    }));
}

function checkProtoAdding() {
    let proto_for_my_object = {
        like_sts: true
    };
    ASSERT_THROWS(TypeError, () => obj.__proto__ = proto_for_my_object);
}

function checkExtensibility() {
    ASSERT_TRUE(Object.isExtensible(obj) === false);
}


checkNewProperty();
checkDeleteProperty();
checkChangeProperty();
checkNewMethod();
checkNewMethodDefineProperty();
checkNewGetterSetter();
checkNewMethodAssign();
checkDeleteMethod();
checkProtoAdding();
checkExtensibility();

