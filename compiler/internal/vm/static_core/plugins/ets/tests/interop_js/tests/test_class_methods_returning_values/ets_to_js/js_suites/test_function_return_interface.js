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
const { getTestModule } = require('../test_class_methods.abc');

const etsMod = getTestModule('class_methods_test');

// Note: Enable when #17878 is resolved
const FIX_17878 = false;

if (FIX_17878) {
	const functionReturnInterfaceEts = etsMod.getFunction('functionReturnInterface');
	{
		const testInterfaceVal = functionReturnInterfaceEts();

		ASSERT_EQ(testInterfaceVal.testNumber, 100);
		ASSERT_EQ(testInterfaceVal.testString, 'Test');
	}
} else {
	const newInterfaceWithMethodEts = etsMod.getFunction('newInterfaceWithMethodEts');

	{
		const testInterfaceVal = newInterfaceWithMethodEts();

		ASSERT_EQ(testInterfaceVal.methodInInterface(), 100);
	}
}
