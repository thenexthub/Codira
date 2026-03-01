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
const TestJSToSpliced = etsMod.getFunction('Array_TestJSToSpliced');

// NOTE(kprokopenko): change to `x.length` when interop support properties
const etsArrLen = (x) => x.length;

// NOTE(oignatenko) enable after interop will be supported for this method signature
const FIXES_IMPLEMENTED = false;

{
	// Test JS Array<FooClass>
	TestJSToSpliced(new Array(new FooClass('zero'), new FooClass('one')));
}

{
	// Test ETS Array<Object>
	let arr = CreateEtsSample();
	const EXPECT_3 = 3;
	arr.push('spliced');
	ASSERT_EQ(etsArrLen(arr), EXPECT_3);
	let toSpliced = arr.toSpliced(1, 1);
	ASSERT_EQ(toSpliced.at(1), 'spliced');
	ASSERT_EQ(etsArrLen(toSpliced), 2);

	if (FIXES_IMPLEMENTED) {
		let arr1 = CreateEtsSample();
		arr1.push('spliced');
		ASSERT_EQ(etsArrLen(arr1), EXPECT_3);
		let toSpliced1 = arr.toSpliced(1);
		ASSERT_EQ(toSpliced1.at(0), 123);
		ASSERT_EQ(toSpliced1.length, 1);
	}
}

GCODESRuntimeCleanup();
