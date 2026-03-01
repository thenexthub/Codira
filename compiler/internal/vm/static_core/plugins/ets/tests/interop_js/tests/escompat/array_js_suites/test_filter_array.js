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
const TestJSFilterArray = etsMod.getFunction('Array_TestJSFilterArray');

{
	// Test JS Array<FooClass>
	TestJSFilterArray(new Array(new FooClass('zero'), new FooClass('one')));
}

{
	// Test ETS Array<Object>
	let arr = CreateEtsSample();
	function fnTrue(v) {
		return true;
	}
	function fn1True(v, k) {
		return true;
	}
	let filter = arr.filterArray(fnTrue);
	let filter1 = arr.filterArray(fn1True);
	ASSERT_EQ(filter[0], arr.at(0));
	ASSERT_EQ(filter1[0], arr.at(0));
	function fnFalse(v) {
		return false;
	}
	function fn1False(v, k) {
		return k < 0;
	}
	let filterFalse = arr.filterArray(fnFalse);
	let filter1False = arr.filterArray(fn1False);
	ASSERT_NE(filterFalse[0], arr.at(0));
	ASSERT_NE(filter1False[0], arr.at(0));
}

GCODESRuntimeCleanup();
