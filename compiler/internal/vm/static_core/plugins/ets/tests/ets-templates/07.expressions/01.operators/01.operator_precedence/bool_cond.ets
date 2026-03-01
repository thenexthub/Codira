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
desc: conditional && and || operators on boolean values
---*/

let res = "";

function t1(): boolean { res += "1"; return true; }
function t2(): boolean { res += "2"; return false; }
function t3(): boolean { res += "3"; return true; }
function t4(): boolean { res += "4"; return false; }

function main() {

    	res = ""
	arktest.assertTrue(t1() && t2() || t3())  // TFT
	arktest.assertEQ(res, "123")

	res = ""
	arktest.assertTrue(t1() || t2() && t3())  // TFT
	arktest.assertEQ(res, "1")

	res = ""
	arktest.assertFalse(t2() || t3() && t4())  // FTF
	arktest.assertEQ(res, "234")

	res = ""
	arktest.assertFalse(t2() && t3() || t4())  // FTF
	arktest.assertEQ(res, "24")

	res = ""
	arktest.assertTrue(!t1() && t2() || t3())  // FFT
	arktest.assertEQ(res, "13")

	res = ""
	arktest.assertFalse(!t1() || t2() && t3())  // FFT
	arktest.assertEQ(res, "12")

	res = ""
	arktest.assertTrue(!t2() || t3() && t4())  // TTF
	arktest.assertEQ(res, "2")

	res = ""
	arktest.assertTrue(!t2() && t3() || t4())  // TTF
	arktest.assertEQ(res, "23")

	res = ""
	arktest.assertTrue(t1() && !t2() || t3())  // TTT
	arktest.assertEQ(res, "12")

	res = ""
	arktest.assertTrue(t1() || !t2() && t3())  // TTT
	arktest.assertEQ(res, "1")

	res = ""
	arktest.assertFalse(t2() || !t3() && t4())  // FFF
	arktest.assertEQ(res, "23")

	res = ""
	arktest.assertFalse(t2() && !t3() || t4())  // FFF
	arktest.assertEQ(res, "24")    
}
