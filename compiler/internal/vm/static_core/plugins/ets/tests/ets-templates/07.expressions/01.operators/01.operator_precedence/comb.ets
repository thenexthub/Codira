/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

/*---
desc: combined multiple operations
---*/

function main() {

	let t1 = 1, t2 = 2, t3 = 1, t4 = 2, t5 = 1, t6 = 2;

	arktest.assertTrue(++t1 + t2++ > t3 % t1 * t6 + t4 | ++t2 / t6-- + ~t1 > t3-- | t3 <= t5 && t3 << t6 / t4 > -t1)
	arktest.assertTrue(t6 + t5 + t4 * ~t3 % ++t1 - t6 != t1)

    
}
