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
	const suite = new arktest.ArkTestsuite("jsonx.fromJson")

	suite.addTest("Test fromJson", testFromJson)
}

function testFromJson() {
	let jsonStr = `
		{
			"foo": 42,
			"bar": "myBar",
			"t2": {
				"newName": false
			}
		}
	`
	const parsed = JSON.parseJsonElement(jsonStr)
	let t1 = new T1
	t1.fromJSON(parsed)
	arktest.assertEQ(t1.foo, 42)
	arktest.assertEQ(t1.bar, "myBar")
	arktest.assertFalse(t1.t2.biz)
}

class T2 implements jsonx.JsonElementDeserializable {
	biz: boolean

	fromJSON(jsonElem: jsonx.JsonElement) {
		this.biz = jsonElem.getBoolean("newName")
	}
}

class T1 implements jsonx.JsonElementDeserializable {
	foo: int
	bar: string
	t2: T2

	constructor() {
		this.foo = 1
		this.bar = "bar"
		this.t2 = new T2
	}

	fromJSON(jsonElem: jsonx.JsonElement) {
		this.foo = jsonElem.getInteger("foo") 
		this.bar = jsonElem.getString("bar")
		let myT2 = new T2
		myT2.fromJSON(jsonElem.getElement("t2"))
		this.t2 = myT2
	}
}
