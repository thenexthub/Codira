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
const ClassImplementingInterfaceWithPropsAndMethod = etsMod.getClass('ClassImplementingInterfaceWithPropsAndMethod');
const functionArgIsInterface = etsMod.getFunction('function_arg_is_interface');

{
	const INT_VALUE = 222;
	const STRING_VALUE = 'hehehe';

	// Method 'foo' invocation from function will change field value
	const CHANGED_INT_VALUE = INT_VALUE * 2;

	// Concatenation of various class field values and '123'
	const RESULT_VALUE = STRING_VALUE + ' ' + CHANGED_INT_VALUE + ' 123';

	const obj = new ClassImplementingInterfaceWithPropsAndMethod(INT_VALUE, STRING_VALUE);

	let ret = functionArgIsInterface(obj);
	ASSERT_EQ(ret, RESULT_VALUE);

	// NOTE replace with direct 'numVal' access after property access will be
	//      implemented for ETS classes
	let changedNumValue = obj.getNumVal();
	ASSERT_EQ(changedNumValue, CHANGED_INT_VALUE);
}
