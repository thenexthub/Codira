/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const { etsVm, getTestModule } = require('../escompat.test.abc');

const etsMod = getTestModule('escompat_test');
const GCODESRuntimeCleanup = etsMod.getFunction('GCODESRuntimeCleanup');
const FooClass = etsMod.getClass('FooClass');
const CreateEtsSample = etsMod.getFunction('Array_CreateEtsSample');
const TestJSFindIndex = etsMod.getFunction('Array_TestJSFindIndex');

{
	// Test JS Array<FooClass>
	TestJSFindIndex(new Array(new FooClass('zero'), new FooClass('one')));
}

{
	// Test ETS Array<Object>
	let arr = CreateEtsSample();
	function fnTrue(v) {
		return true;
	}
	function fnFalse(v) {
		return false;
	}

	let found = arr.findIndex(fnTrue);
	ASSERT_EQ(found, 0);
	let foundStatic = Array.findIndex(fnTrue, arr);
	ASSERT_EQ(foundStatic, 'foo');

	let foundNot = arr.findIndex(fnFalse);
	ASSERT_TRUE(foundNot === -1);
	let foundNotStatic = Array.findIndex(fnFalse, arr);
	ASSERT_TRUE(foundNotStatic === -1);
}

GCODESRuntimeCleanup();
