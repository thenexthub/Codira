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

import { jsonx } from "std/core"

function main() {
	const suite = new arktest.ArkTestsuite("jsonx.toJson")

	suite.addTest("Test toJson", testToJson)
}

function testToJson() {
	let t1 = new T1
	let jsonedT1 = t1.toJSON()
	arktest.assertEQ(jsonedT1.getString("newBar"), "34")
	arktest.assertEQ(jsonedT1.getInteger("foo"), 5)
	let jsonedT2 = jsonedT1.getElement("t2")
	arktest.assertTrue(jsonedT2.getBoolean("biz"))
}

class T2 implements jsonx.JsonElementSerializable {
	biz: boolean = true

	toJSON(): jsonx.JsonElement {
		let val = jsonx.JsonElement.createBoolean(this.biz)
		let m: Record<string, jsonx.JsonElement> = {
			"biz": val,
		}
		return jsonx.JsonElement.createObject(m)
	}
}

class T1 implements jsonx.JsonElementSerializable {
	foo: int = 5
	bar: string = "34"
	t2: T2 = new T2

	toJSON(): jsonx.JsonElement {
		let jFoo = jsonx.JsonElement.createInteger(this.foo)
		let jBar = jsonx.JsonElement.createString(this.bar)
		let jT2 = this.t2.toJSON()

		let m: Record<string, jsonx.JsonElement> = {
			"foo": jFoo,
			"newBar": jBar,
			"t2": jT2,
		}
		return jsonx.JsonElement.createObject(m)
	}
}
