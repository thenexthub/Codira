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
const TestJSReduce = etsMod.getFunction('Array_TestJSReduce');

{
	// Test JS Array<FooClass>
	TestJSReduce(new Array(new FooClass('zero'), new FooClass('one')));
}

{
	// Test ETS Array<Object>
	let arr = CreateEtsSample();

	function fnReduce(a, b) {
		return a;
	}
	let reduced = arr.reduce(fnReduce);
	ASSERT_EQ(reduced, arr.at(0));

	function fnReduce2(a, b) {
		return b;
	}
	let reduced2 = arr.reduce(fnReduce2, 'initVal');
	ASSERT_EQ(reduced, arr.at(1));
}

GCODESRuntimeCleanup();
