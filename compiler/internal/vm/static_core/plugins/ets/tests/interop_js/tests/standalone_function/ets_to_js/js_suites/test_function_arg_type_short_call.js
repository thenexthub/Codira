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
const { etsVm, getTestModule } = require('scenarios.test.js');

const etsMod = getTestModule('interopStandaloneFnTest');
const functionArgTypeShortEts = etsMod.getFunction('functionArgTypeShortEts');

{
	const EXPECTED_POSITIVE_SHORT = 32767;
	const EXPECTED_NEGATIVE_SHORT = -32768;
	const EXCEEDED_VALUE = 32768;
	ASSERT_EQ(functionArgTypeShortEts(EXPECTED_POSITIVE_SHORT), EXPECTED_POSITIVE_SHORT);
	ASSERT_EQ(functionArgTypeShortEts(EXPECTED_NEGATIVE_SHORT), EXPECTED_NEGATIVE_SHORT);
	ASSERT_THROWS(TypeError, () => functionArgTypeShortEts(EXCEEDED_VALUE));
}
