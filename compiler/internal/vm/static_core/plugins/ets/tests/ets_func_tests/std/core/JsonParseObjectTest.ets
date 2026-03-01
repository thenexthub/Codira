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

const OBJ_JSON =  '{"name":"foo"}'

function testJsonParseObjectAsString(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<string>())
	}, 'std.core.String is expected, but get std.core.JSONObject')
}

function testJsonParseObjectAsInt(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<Int>())
	}, 'std.core.Int is expected, but get std.core.JSONObject')
}

function testJsonParseObjectAsChar(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<Char>())
	}, 'std.core.Char is expected, but get std.core.JSONObject')
}

function testJsonParseObjectAsPlainObject(): void {
	const obj = JSON.parse<object>(OBJ_JSON, Class.from<object>())
	arktest.assertTrue(obj instanceof Object)
}

interface Named {
	name: string
}

function testJsonParseObjectAsInterface(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<Named>())
	}, "Incorrect type: 'JsonParseObjectTest.Named' doesn't provide default public constructor")
}

class NamedRef {
	ref: Named | undefined = undefined
}

function testJsonParseObjectAsClassWithInterfaceField(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<NamedRef>('{}', Class.from<NamedRef>())
	}, "Incorrect field: 'JsonParseObjectTest.NamedRef.ref'. Incorrect type: 'JsonParseObjectTest.Named' doesn't provide default public constructor")
}

class SubArray<T> extends Array<T> {}

function testJsonParseObjectAsArray(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<Array<object>>())
	}, 'std.core.Array is expected, but get std.core.JSONObject')

	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<SubArray<object>>())
	}, 'JsonParseObjectTest.SubArray is expected, but get std.core.JSONObject')
}

function testJsonParseObjectAsFixedArray(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<FixedArray<object>>())
	}, 'std.core.Object[] is expected, but get std.core.JSONObject')

	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<FixedArray<int>>())
	}, 'i32[] is expected, but get std.core.JSONObject')
}

function testJsonParseObjectAsNull(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<null>())
	}, 'std.core.Null is expected, but get std.core.JSONObject')
}

function testJsonObjectAsFunction(): void {
	arktest.expectError(() => {
		const obj = JSON.parse<object>(OBJ_JSON, Class.from<() => void>())
	}, "Incorrect type: 'std.core.Function0' doesn't provide default public constructor")
}

function main(): int {
	const suite = new arktest.ArkTestsuite('JSON.parse Object')
	suite.addTest('as String', testJsonParseObjectAsString)
	suite.addTest('as Int', testJsonParseObjectAsInt)
	suite.addTest('as Char', testJsonParseObjectAsChar)
	suite.addTest('as interface', testJsonParseObjectAsInterface)
	suite.addTest('as class with interface field', testJsonParseObjectAsClassWithInterfaceField)
	suite.addTest('as Array', testJsonParseObjectAsArray)
	suite.addTest('as FixedArray', testJsonParseObjectAsFixedArray)
	suite.addTest('as null', testJsonParseObjectAsNull)
	suite.addTest('as function', testJsonObjectAsFunction)
	suite.addTest('as plain Object', testJsonParseObjectAsPlainObject)
	return suite.run()
}
