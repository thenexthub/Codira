/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

const etsVm = globalThis.gtest.etsVm;

const PropertyName = 'hello';
const ClassObjSetterHello = 'ClassObjSetter hello';
const LiteralObjSetterHello = 'LiteralObjSetter hello';
const DefinePropertySetterHello = 'DefinePropertySetter hello';
const ComputedPropertySetterHello = 'ComputedPropertySetter hello';
const callObjSetter = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'callObjSetter');

function testClassObjSetter() {
    class ClassObjSetter {
        helloStr;
        set hello(val) {
            this.helloStr = val;
        }
    }
    const GClass = new ClassObjSetter();
    callObjSetter(GClass, ClassObjSetterHello);
    ASSERT_TRUE(GClass.helloStr === ClassObjSetterHello);
}

function testLiteralObjSetter() {
    let LiteralObjSetter = {
        helloStr: null,
        set hello(val) {
            this.helloStr = val;
        }
    };
    callObjSetter(LiteralObjSetter, LiteralObjSetterHello);
    ASSERT_TRUE(LiteralObjSetter.helloStr === LiteralObjSetterHello);
}

function testDeleteLiteralObjSetter() {
    let LiteralObjSetter = {
        helloStr: null,
        set hello(val) {
            this.helloStr = val;
        }
    };
    delete LiteralObjSetter.hello;
    callObjSetter(LiteralObjSetter, LiteralObjSetterHello);
    ASSERT_TRUE(LiteralObjSetter.helloStr === null);
    ASSERT_TRUE(LiteralObjSetter.hello === LiteralObjSetterHello);
}

function testDefinePropertySetter() {
    let obj = { helloStr: null };
    Object.defineProperty(obj, PropertyName, {
        set: function (val) {
            this.helloStr = val;
        }
    });
    callObjSetter(obj, DefinePropertySetterHello);
    ASSERT_TRUE(obj.helloStr === DefinePropertySetterHello);
}

function testComputedPropertySetter() {
    let propName = PropertyName;
    let LiteralObjSetter = {
        helloStr: null,
        set [propName](val) {
            this.helloStr = val;
        }
    };
    callObjSetter(LiteralObjSetter, ComputedPropertySetterHello);
    ASSERT_TRUE(LiteralObjSetter.helloStr === ComputedPropertySetterHello);
}


testClassObjSetter();
testLiteralObjSetter();
testDeleteLiteralObjSetter();
testDefinePropertySetter();
testComputedPropertySetter();