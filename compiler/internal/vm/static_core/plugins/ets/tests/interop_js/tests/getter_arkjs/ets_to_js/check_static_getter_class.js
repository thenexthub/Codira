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

import { string, number, bool, arr, obj, tuple, PUBLIC_STATIC_GETTER_CLASS_VALUE } from './constant';

const etsVm = globalThis.gtest.etsVm;
const checkArray = (arg) => arg instanceof Array;
const checkObj = (arg) => arg !== null && typeof arg === 'object' && !Array.isArray(arg);
const StaticClass = etsVm.getClass('Lgetter/test/StaticClass;');
const StaticPublicFieldClass = etsVm.getClass('Lgetter/test/StaticPublicFieldClass;');


function checkClassGetterClassValueInt() {
	const GClass = new StaticClass(number);
	ASSERT_TRUE(GClass.value === number);
}

function checkClassGetterClassValueString() {
	const GClass = new StaticClass(string);
	ASSERT_TRUE(GClass.value === string);
}

function checkClassGetterClassValueBool() {
	const GClass = new StaticClass(bool);
	ASSERT_TRUE(GClass.value === bool);
}

function checkClassGetterClassValueArr() {
	const GClass = new StaticClass(arr);
	ASSERT_TRUE(checkArray(GClass.value) && GClass.value[0] === arr[0]);
}

function checkClassGetterClassValueObj() {
	const GClass = new StaticClass(obj);
	ASSERT_TRUE(checkObj(GClass.value));
}

function checkClassGetterClassValueTuple() {
	const GClass = new StaticClass(tuple);
	const res = GClass.value;
	ASSERT_TRUE(checkArray(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function checkClassStaticGetterSetter() {
	StaticClass.value = PUBLIC_STATIC_GETTER_CLASS_VALUE;
	ASSERT_TRUE(StaticClass.value === PUBLIC_STATIC_GETTER_CLASS_VALUE);
}

function checkClassStaticPublicField() {
	const GClass = new StaticPublicFieldClass(string);
	ASSERT_TRUE(GClass.value === string);
	StaticPublicFieldClass.value = PUBLIC_STATIC_GETTER_CLASS_VALUE;
	ASSERT_TRUE(StaticPublicFieldClass.value === PUBLIC_STATIC_GETTER_CLASS_VALUE);
}


checkClassGetterClassValueInt();
checkClassGetterClassValueString();
checkClassGetterClassValueBool();
checkClassGetterClassValueArr();
checkClassGetterClassValueObj();
checkClassGetterClassValueTuple();
checkClassStaticGetterSetter();
checkClassStaticPublicField();