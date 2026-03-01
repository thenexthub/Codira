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

function dummy(a: int, b: string): void {}

function testParameterTypes(): void {
	const ft = Type.of(dummy) as FunctionType
	arktest.assertEQ(2, ft.getParametersNum())
	const p0 = ft.getParameter(0)
	const p1 = ft.getParameter(1)
	arktest.assertTrue(p0.getType().equals(IntType.VAL))
	arktest.assertTrue(p1.getType().equals(StringType.REF))
}

function main(): int {
	const suite = new arktest.ArkTestsuite('Parameter mirror API tests for SDK')
	suite.addTest('Parameter.getType', testParameterTypes)
	return suite.run()
}
