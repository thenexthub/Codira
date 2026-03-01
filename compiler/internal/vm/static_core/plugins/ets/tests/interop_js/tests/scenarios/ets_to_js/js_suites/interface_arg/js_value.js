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
const { etsVm, getTestModule } = require('../../scenarios.test.abc');

const etsMod = getTestModule('scenarios_test');
const functionArgIsInterface = etsMod.getFunction('function_arg_is_interface');

class ClassImplementingInterfaceWithPropsAndMethod {
	numVal = 0;
	stringVal = 'hehehe';

	foo(x) {
		let result = this.stringVal + ' ' + this.numVal + ' ' + x;
		return result;
	}
}

{
	const INT_VALUE = 222;

	// Method 'foo' invocation from function will change field value
	const CHANGED_INT_VALUE = INT_VALUE * 2;

	// Concatenation of various class field values and '123'
	const RESULT_VALUE = 'hehehe ' + CHANGED_INT_VALUE + ' 123';

	const obj = new ClassImplementingInterfaceWithPropsAndMethod();
	obj.numVal = INT_VALUE;

	let ret = functionArgIsInterface(obj);
	ASSERT_EQ(ret, RESULT_VALUE);

	let changedNumValue = obj.numVal;
	ASSERT_EQ(changedNumValue, CHANGED_INT_VALUE);
}
