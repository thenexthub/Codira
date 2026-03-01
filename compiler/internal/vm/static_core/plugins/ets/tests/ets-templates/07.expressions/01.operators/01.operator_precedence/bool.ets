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
desc: operators on boolean values
---*/

function main() {

	let v = false, vt = true, vf = false;

	// & |
	v = vt | vf & vf
	arktest.assertTrue(v)

	v = vt | vt & vf
	arktest.assertTrue(v)

	// & ^
	v = vt ^ vf & vf
	arktest.assertTrue(v)

	v = vt ^ vt & vf
	arktest.assertTrue(v)

	// ^ |
	v = vt | vf ^ vt
	arktest.assertTrue(v)

	v = vt | vt ^ vt
	arktest.assertTrue(v)

	// & ==
	v = vf & vf == vf
	arktest.assertFalse(v)

	v = vf & vt == vf
	arktest.assertFalse(v)

	// & !=
	v = vf & vf != vt
	arktest.assertFalse(v)

	v = vf & vt != vt
	arktest.assertFalse(v)

	// | ==
	v = vt | vf == vf
	arktest.assertTrue(v)

	v = vt | vt == vf
	arktest.assertTrue(v)

	// | !=
	v = vt | vf != vt
	arktest.assertTrue(v)

	v = vt | vt != vt
	arktest.assertTrue(v)
    
}
