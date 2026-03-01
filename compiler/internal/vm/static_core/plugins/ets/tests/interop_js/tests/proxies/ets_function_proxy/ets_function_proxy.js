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

const { getTestFunction } = require('ets_function_proxy.test.abc');

{
	// test simple function-proxy capabilities
	const sumDouble = getTestFunction('sum_double');

	ASSERT_TRUE(Object.isSealed(sumDouble));
	ASSERT_EQ(sumDouble(3, 4), 3 + 4);

	ASSERT_THROWS(TypeError, () => sumDouble(1));
	ASSERT_THROWS(TypeError, () => sumDouble(1, 2, 3, 4));
	ASSERT_THROWS(TypeError, () => sumDouble(1, true));
}

{
	// test object-proxy
	const pointCreate = getTestFunction('point_create');
	const pointGetX = getTestFunction('point_get_x');
	const pointGetY = getTestFunction('point_get_y');

	let p = pointCreate(2, 6);
	let x = pointGetX(p);
	let y = pointGetY(p);

	ASSERT_EQ(x, 2);
	ASSERT_EQ(y, 6);
}
