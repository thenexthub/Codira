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
desc: relational and logical operations
---*/

function main() {
	let t1 = 1, t2 = 2, t3 = 3, t4 = 4;

	arktest.assertTrue(t1 > t2 || t2 >= t3 && t3 < t4 || t4 <= t1 && t4 == t2 || t4 != t1)
	arktest.assertFalse(t1 > t2 && t2 >= t3 || t3 < t4 && t4 <= t1 || t4 == t2 && t4 != t1)

	// prefix and postfix added
	arktest.assertTrue(++t1 <= t2-- && --t2 == t3++ && ++t3 != t4-- || --t4 <= t1++ && ++t4 == t2-- || --t4 != t1++)
	arktest.assertFalse(++t1 <= t2-- || --t2 == t3++ && ++t3 != t4-- && --t4 <= t1++ || ++t4 == t2-- && --t4 != t1++)

	// bitwise xor, unary + and - added
	arktest.assertTrue(++t1 <= ~t2-- && --t2 == -t3++ && ++t3 != -t4-- 
                        || --t4 <= ~t1++ && ++t4 == +t2-- || --t4 != -t1++)
	arktest.assertFalse(++t1 <= -~++t2 && ~++t2 == -t3++ && ++t3 != -t4-- 
                        || ~+t4++ <= ~t1++ && -t4 == +t2-- || --t4 == -t1++)

    
}
