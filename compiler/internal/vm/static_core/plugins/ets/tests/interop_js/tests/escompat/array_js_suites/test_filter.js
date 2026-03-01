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
const TestJSFilter = etsMod.getFunction('Array_TestJSFilter');

{
	// Test JS Array<FooClass>
	TestJSFilter(new Array(new FooClass('zero'), new FooClass('one')));
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
	let filter = arr.filter(fnTrue);
	let filter1 = arr.filter(fn1True);
	ASSERT_EQ(filter.at(0), arr.at(0));
	ASSERT_EQ(filter1.at(0), arr.at(0));
	function fnFalse(v) {
		return false;
	}
	function fn1False(v, k) {
		return k < 0;
	}
	let filterFalse = arr.filter(fnFalse);
	let filter1False = arr.filter(fn1False);
	ASSERT_NE(filterFalse.at(0), arr.at(0));
	ASSERT_NE(filter1False.at(0), arr.at(0));
}

GCODESRuntimeCleanup();
