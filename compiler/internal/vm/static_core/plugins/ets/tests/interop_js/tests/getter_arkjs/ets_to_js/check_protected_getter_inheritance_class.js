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

import {
	PROTECTED_GETTER_CLASS_VALUE,
	PROTECTED_GETTER_OVERRIDE_CLASS_VALUE
 } from './constant';

const etsVm = globalThis.gtest.etsVm;
const ProtectedGetterInheritanceClass = etsVm.getClass('Lgetter/test/ProtectedGetterInheritanceClass;');
const ProtectedGetterOverrideClass = etsVm.getClass('Lgetter/test/ProtectedGetterOverrideClass;');
const createProtectedGetterInheritanceClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_protected_getter_inheritance_class_from_ets');
const createProtectedGetterOverrideClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_protected_getter_override_class_from_ets');


function checkProtectedGetterInheritanceClassalue() {
	const GClass = new ProtectedGetterInheritanceClass();
	ASSERT_TRUE(GClass.alue === undefined);
}

function checkProtectedGetterInheritanceClass() {
	const GClass = new ProtectedGetterInheritanceClass();
	ASSERT_TRUE(GClass.value === PROTECTED_GETTER_CLASS_VALUE);
}

function checkCreateProtectedetterInheritanceClassFromEts() {
	const GClass = createProtectedGetterInheritanceClassFromEts();
	ASSERT_TRUE(GClass.alue === undefined);
}

function checkCreateProtectedGetterInheritanceClassFromEts() {
	const GClass = createProtectedGetterInheritanceClassFromEts();
	ASSERT_TRUE(GClass.value === PROTECTED_GETTER_CLASS_VALUE);
}

function checkProtectedGetterOverrideClass() {
	const GClass = new ProtectedGetterOverrideClass();
	ASSERT_TRUE(GClass.value === PROTECTED_GETTER_OVERRIDE_CLASS_VALUE);
}

function checkCreateProtectedetterOverrideClassFromEts() {
	const GClass = createProtectedGetterOverrideClassFromEts();
	ASSERT_TRUE(GClass.value === PROTECTED_GETTER_OVERRIDE_CLASS_VALUE);
}

checkProtectedGetterInheritanceClassalue();
checkProtectedGetterInheritanceClass();
checkCreateProtectedetterInheritanceClassFromEts();
checkCreateProtectedGetterInheritanceClassFromEts();
checkProtectedGetterOverrideClass();
checkCreateProtectedetterOverrideClassFromEts();