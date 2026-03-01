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

import { string, number, bool, arr, obj } from './constant';

const etsVm = globalThis.gtest.etsVm;
const checkArray = (arg) => arg instanceof Array;
const checkObj = (arg) => arg !== null && typeof arg === 'object' && !Array.isArray(arg);
const AnyTypeClass = etsVm.getClass('Lgetter/test/AnyTypeClass;');
const createAnyTypeGetterClassFromEtsString = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_string');
const createAnyTypeGetterClassFromEtsInt = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_int');
const createAnyTypeGetterClassFromEtsBool = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_bool');
const createAnyTypeGetterClassFromEtsArr = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_arr');
const createAnyTypeGetterClassFromEtsObj = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_obj');
const createAnyTypeGetterClassFromEtsUnion = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_union');


function checkClassGetterClassValueInt() {
    const GClass = new AnyTypeClass(number);
    ASSERT_TRUE(GClass.value === number);
}

function checkClassGetterClassValueString() {
    const GClass = new AnyTypeClass(string);
    ASSERT_TRUE(GClass.value === string);
}

function checkClassGetterClassValueBool() {
    const GClass = new AnyTypeClass(bool);
    ASSERT_TRUE(GClass.value === bool);
}

function checkClassGetterClassValueArr() {
    const GClass = new AnyTypeClass(arr);
    ASSERT_TRUE(checkArray(GClass.value) && GClass.value[0] === arr[0]);
}

function checkClassGetterClassValueObj() {
    const GClass = new AnyTypeClass(obj);
    ASSERT_TRUE(checkObj(GClass.value));
}

function checkCreateAnyTypeGetterClassFromEtsString() {
    const GClass = createAnyTypeGetterClassFromEtsString(string);
    ASSERT_TRUE(GClass.value === string);
}

function checkCreateAnyTypeGetterClassFromEtsInt() {
    const GClass = createAnyTypeGetterClassFromEtsInt(number);
    ASSERT_TRUE(GClass.value === number);
}

function checkCreateAnyTypeGetterClassFromEtsBool() {
    const GClass = createAnyTypeGetterClassFromEtsBool(bool);
    ASSERT_TRUE(GClass.value === bool);
}

function checkCreateAnyTypeGetterClassFromEtsArr() {
    const GClass = createAnyTypeGetterClassFromEtsArr(arr);
    ASSERT_TRUE(checkArray(GClass.value) && GClass.value[0] === arr[0]);
}

function checkCreateAnyTypeGetterClassFromEtsObj() {
    const GClass = createAnyTypeGetterClassFromEtsObj(obj);
    ASSERT_TRUE(checkObj(GClass.value));
}

function checkCreateAnyTypeGetterClassFromEtsUnion() {
    const GClass = createAnyTypeGetterClassFromEtsUnion(number);
    ASSERT_TRUE(GClass.value === number);
}

checkClassGetterClassValueInt();
checkClassGetterClassValueString();
checkClassGetterClassValueBool();
checkClassGetterClassValueArr();
checkClassGetterClassValueObj();
checkCreateAnyTypeGetterClassFromEtsString();
checkCreateAnyTypeGetterClassFromEtsInt();
checkCreateAnyTypeGetterClassFromEtsBool();
checkCreateAnyTypeGetterClassFromEtsArr();
checkCreateAnyTypeGetterClassFromEtsObj();
checkCreateAnyTypeGetterClassFromEtsUnion();
