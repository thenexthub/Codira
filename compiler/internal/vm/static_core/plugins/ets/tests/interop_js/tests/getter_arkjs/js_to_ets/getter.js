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
const ClassObjGetterHello = 'ClassObjGetter hello';
const LiteralObjGetterHello = 'LiteralObjGetter hello';
const DefinePropertyGetterHello = 'DefinePropertyGetter hello';
const ComputedPropertyGetterHello = 'ComputedPropertyGetter hello';

class ClassObjGetter {
    get hello() {
        return ClassObjGetterHello;
    }
}

const checkClassGetter = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'checkClassGetter');
const checkLiteralObjGetter = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'checkLiteralObjGetter');
const checkDeleteLiteralObjGetter = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'checkDeleteLiteralObjGetter');
const checkDefinePropertyGetter = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'checkDefinePropertyGetter');
const checkComputedPropertyGetter = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'checkComputedPropertyGetter');

function testClassObjGetter() {
    const GClass = new ClassObjGetter();
    ASSERT_TRUE(checkClassGetter(GClass, ClassObjGetterHello));
}

function testLiteralObjGetter() {
    let LiteralObjGetter = {
        get hello() {
            return LiteralObjGetterHello;
        }
    };
    ASSERT_TRUE(checkLiteralObjGetter(LiteralObjGetter, LiteralObjGetterHello));
}

function testDeleteLiteralObjGetter() {
    let LiteralObjGetter = {
        get hello() {
            return LiteralObjGetterHello;
        }
    };
    delete LiteralObjGetter.hello;
    ASSERT_TRUE(checkDeleteLiteralObjGetter(LiteralObjGetter));
}

function testDefinePropertyGetter() {
    let obj = {};
    Object.defineProperty(obj, PropertyName, {
        get: function () {
            return DefinePropertyGetterHello;
        }
    });
    ASSERT_TRUE(checkDefinePropertyGetter(obj, DefinePropertyGetterHello));
}

function testComputedPropertyGetter() {
    let propName = PropertyName;
    let LiteralObjGetter = {
        get [propName]() {
            return ComputedPropertyGetterHello;
        }
    };
    ASSERT_TRUE(checkComputedPropertyGetter(LiteralObjGetter, ComputedPropertyGetterHello));
}


testClassObjGetter();
testLiteralObjGetter();
testDeleteLiteralObjGetter();
testDefinePropertyGetter();
testComputedPropertyGetter();