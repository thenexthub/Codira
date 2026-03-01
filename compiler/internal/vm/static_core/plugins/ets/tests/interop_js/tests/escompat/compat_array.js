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

const { etsVm, getTestModule } = require('escompat.test.abc');

const etsod = getTestModule('escompat_test');
const GCODESRuntimeCleanup = etsod.getFunction('GCODESRuntimeCleanup');
const FooClass = etsod.getClass('FooClass');
const CreateEtsSample = etsod.getFunction('Array_CreateEtsSample');
const TestJSSample = etsod.getFunction('Array_TestJSSample');

{
	// Test JS Array<FooClass>
	TestJSSample(new Array(new FooClass('zero'), new FooClass('one')));
}

{
	// Test ETS Array<Object>
	let arr = CreateEtsSample();
	ASSERT_TRUE(arr instanceof Array);

	ASSERT_EQ(arr.at(0), 123);
	ASSERT_EQ(arr.at(1), 'foo');

	let something = {};
	arr.push(something);
	ASSERT_EQ(arr.at(2), something);
}

GCODESRuntimeCleanup();
