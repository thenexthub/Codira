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

abstract class TagBase {
	val: string

	constructor(val: string) {
		this.val = val
	}
}

class ObjTag extends TagBase {
	constructor(val: string = "") {
		super(val)
	}
}

class Obj {
	id: int
	name: string | undefined

	next: Obj | undefined

	tag: ObjTag | undefined

	b: boolean = true
	bt: byte = 0
	c: char = c'a'
	s: short = 0
	l: long = 0
	f: float = 0.0f
	d: double = 0.0

	constructor(id: int = 0, name: string | undefined = undefined) {
		this.id = id
		this.name = name
	}
}

class RefCycle {
	ref: RefCycle | undefined
}

class FnBox {
	fn: (() => boolean) | undefined = undefined
}

function testObjectClone(): void {
	const obj = new Obj(1, "foo")
	obj.tag = new ObjTag("object")

	const objCopy = deepcopy(obj)
	arktest.assertTrue(obj !== objCopy, "cloned object is the same as source object")
	arktest.assertTrue(objCopy != null, "cloned object is null")

	arktest.assertEQ(obj.id,  objCopy!.id, "cloned object 'id' field unexpected value")
	arktest.assertEQ(obj.name,  objCopy!.name, "cloned object 'name' field unexpected value")

	arktest.assertTrue(objCopy!.next === undefined, "cloned object 'next' field unexpected value")
	arktest.assertTrue(obj.tag !== objCopy!.tag, "object tag is not cloned")
	arktest.assertEQ(obj.tag!.val, objCopy!.tag!.val, "cloned object tag value has unexpected value")
}

function testObjectShallowCopy(): void {
	const obj = new Obj(2, "bar")
	obj.tag = new ObjTag("shallow")

	const objCopy = deepcopy(obj, 1)
	arktest.assertTrue(obj !== objCopy, "cloned object is the same as source object")
	arktest.assertTrue(objCopy != null, "cloned object is null")

	arktest.assertTrue(obj.tag === objCopy!.tag, "object tag is cloned also")
}

function testObjectNoCopy(): void {
	const obj = new Obj(3, "spam")
	obj.tag = new ObjTag("no copy")

	const objCopy = deepcopy(obj, 0)
	arktest.assertTrue(obj === objCopy, "returned object should be same as the source object")
}

function testObjectCloneDepth2(): void {
	const obj = new Obj(4, "eggs")
	obj.tag = new ObjTag("depth 2")

	const next = new Obj(5, "nest")
	next.tag = new  ObjTag("nested")

	obj.next = next

	const objCopy = deepcopy(obj, 2)

	arktest.assertTrue(objCopy != null, "cloned object is null")
	arktest.assertTrue(obj !== objCopy, "cloned object is the same as source")

	arktest.assertTrue(obj.next !== objCopy!.next, "one level nested object is the same as source")
	arktest.assertTrue(obj.next!.tag === objCopy!.next!.tag, "2 level nested objects should not be cloned")
}

function testObjectRefCycleClone(): void {
	const ref = new RefCycle()
	ref.ref = ref

	const refCopy = deepcopy(ref)

	arktest.assertTrue(ref === ref.ref, "not a reference cycle")
	arktest.assertTrue(ref !== refCopy, "source object returned")
	arktest.assertTrue(refCopy != null, "null returned")

	arktest.assertTrue(refCopy === refCopy!.ref, "cloned object has broken reference cycle")
}

function testBuiltinArrayClone(): void {
	const obj = new Obj()
	const objs: Obj[] = [obj, obj]

	const objsCopy = deepcopy(objs)
	arktest.assertTrue(objsCopy != null, "cloned array is null")

	arktest.assertEQ(objs.length,  objsCopy!.length, "cloned array invalid length")
	arktest.assertTrue(objs !== objsCopy!, "source array returned")
	arktest.assertTrue(objs[0] !== objsCopy![0], "array elements are not cloned")
	arktest.assertTrue(objsCopy![0] === objsCopy![1], "cloned array elements are different")
}

function testBoolArrayClone(): void {
	const bools: FixedArray<boolean> = [ true ]
	const boolsCopy = deepcopy(bools)

	arktest.assertTrue(boolsCopy != null, "cloned array is null")
	arktest.assertTrue(bools !== boolsCopy, "source array returned")
	arktest.assertTrue(bools.length == boolsCopy!.length, "original and cloned array have different lengths")
	arktest.assertTrue(bools[0] == boolsCopy![0], "original and cloned array have different elements")
}

function testByteArrayClone(): void {
	const bytes: FixedArray<byte> = [ 1 ]
	const bytesCopy = deepcopy(bytes)

	arktest.assertTrue(bytesCopy != null, "cloned array is null")
	arktest.assertTrue(bytes !== bytesCopy, "source array returned")
	arktest.assertTrue(bytes.length == bytesCopy!.length, "original and cloned array have different lengths")
	arktest.assertTrue(bytes[0] == bytesCopy![0], "original and cloned array have different elements")
}

function testCharArrayClone(): void {
	const chars: FixedArray<char> = [ c'a' ]
	const charsCopy = deepcopy(chars)

	arktest.assertTrue(charsCopy != null, "cloned array is null")
	arktest.assertTrue(chars !== charsCopy, "source array returned")
	arktest.assertTrue(chars.length == charsCopy!.length, "original and cloned array have different lengths")
	arktest.assertTrue(chars[0] == charsCopy![0], "original and cloned array have different elements")
}

function testShortArrayClone(): void {
	const shorts: FixedArray<short> = [ 1 ]
	const shortsCopy = deepcopy(shorts)

	arktest.assertTrue(shortsCopy != null, "cloned array is null")
	arktest.assertTrue(shorts !== shortsCopy, "source array returned")
	arktest.assertTrue(shorts.length == shortsCopy!.length, "original and cloned array have different lengths")
	arktest.assertTrue(shorts[0] == shortsCopy![0], "original and cloned array have different elements")
}

function testIntArrayClone(): void {
	const ints: FixedArray<int> = [ 1 ]
	const intsCopy = deepcopy(ints)

	arktest.assertTrue(intsCopy != null, "cloned array is null")
	arktest.assertTrue(ints !== intsCopy, "source array returned")
	arktest.assertTrue(ints.length == intsCopy!.length, "original and cloned array have different lengths")
	arktest.assertTrue(ints[0] == intsCopy![0], "original and cloned array have different elements")
}

function testLongArrayClone(): void {
	const longs: FixedArray<long> = [ 1 ]
	const longsCopy = deepcopy(longs)

	arktest.assertTrue(longsCopy != null, "cloned array is null")
	arktest.assertTrue(longs !== longsCopy, "source array returned")
	arktest.assertTrue(longs.length == longsCopy!.length, "original and cloned array have different lengths")
	arktest.assertTrue(longs[0] == longsCopy![0], "original and cloned array have different elements")
}

function testFloatArrayClone(): void {
	const floats: FixedArray<float> = [ 1.0 ]
	const floatsCopy = deepcopy(floats)

	arktest.assertTrue(floatsCopy != null, "cloned array is null")
	arktest.assertTrue(floats !== floatsCopy, "source array returned")
	arktest.assertTrue(floats.length == floatsCopy!.length, "original and cloned array have different lengths")
	arktest.assertTrue(floats[0] == floatsCopy![0], "original and cloned array have different elements")
}

function testDoubleArrayClone(): void {
	const nums: FixedArray<double> = [ 1.0 ]
	const numsCopy = deepcopy(nums)

	arktest.assertTrue(numsCopy != null, "cloned array is null")
	arktest.assertTrue(nums !== numsCopy, "source array returned")
	arktest.assertTrue(nums.length == numsCopy!.length, "original and cloned array have different lengths")
	arktest.assertTrue(nums[0] == numsCopy![0], "original and cloned array have different elements")
}

function testArrayClone(): void {
	const objs = new Array<Obj | null | undefined>()
	objs.push(new Obj())
	objs.push(null)
	objs.push(undefined)

	const objsCopy = deepcopy(objs)
	arktest.assertTrue(objsCopy != null, "cloned array is null")

	arktest.assertEQ(objs.length,  objsCopy!.length, "cloned array length is not the as source length")
	arktest.assertTrue(objs !== objsCopy, "cloned array is the same as source array")
	arktest.assertTrue(objs[0] !== objsCopy![0], "array elements are not cloned")
	arktest.assertTrue(objs[1] === null, "null element copied incorrectly")
	arktest.assertTrue(objs[2] === undefined, "undefined element copied incorrectly")
}

function testObjectWithLambdaClone(): void {
	const fnBox = new FnBox()
	fnBox.fn = () => true

	const fnBoxCopy = deepcopy(fnBox)
	arktest.assertTrue(fnBoxCopy != null, "cloned object is null")

	arktest.assertTrue(fnBox !== fnBoxCopy, "cloned object is the same as source object")
	arktest.assertTrue(fnBox.fn === fnBoxCopy!.fn, "lambda field is cloned")
	arktest.assertEQ(fnBox.fn!(),  fnBoxCopy!.fn!(), "not the same results")
}

function testSetClone(): void {
	const set = new Set<Obj>()

	const obj = new Obj()
	set.add(obj)

	const setCopy = deepcopy(set)
	arktest.assertTrue(setCopy != null, "cloned set is null")

	arktest.assertTrue(set !== setCopy, "cloned set is the same as source set")
	arktest.assertEQ(set.size,  setCopy!.size, "cloned set size is not the same as source set size")
	arktest.assertFalse(setCopy!.has(obj), "cloned set contains original element")
}

function testMapClone(): void {
	const map = new Map<string, number | null | undefined>()

	const key = "1.0"
	map.set(key, 1.0)
	map.set("null", null)
	map.set("undefined", undefined)

	const mapCopy = deepcopy(map)
	arktest.assertTrue(mapCopy != null, "cloned map is null")

	arktest.assertTrue(map != mapCopy, "cloned map is the same as source map")
	arktest.assertEQ(map.size,  mapCopy!.size, "cloned map size != source map size")
	arktest.assertTrue(mapCopy!.has(key), "existing key not found in cloned map")

	arktest.assertTrue(mapCopy!.get("null") === null, "'null' key has wrong value")
	arktest.assertTrue(mapCopy!.get("undefined") === undefined, "'undefined' key has wrong value")
}

function testDateClone(): void {
	const date = new Date()

	const dateCopy = deepcopy(date)
	arktest.assertTrue(dateCopy != null, "cloned date is null")

	arktest.assertTrue(date != dateCopy, "cloned date is the same as source date")
	arktest.assertEQ(date.valueOf(),  dateCopy!.valueOf(), "cloned date value is not equal to source date value")
}

function testWeakRefClone(): void {
	const obj = new Obj(6, "ref")
	const ref = new WeakRef<Obj>(obj)

	arktest.expectError(() => { 
		const refCopy = deepcopy(ref) 
	}, "class std.core.WeakRef doesn't have default constructor")
}

async function fn(): Promise<string> {
	return "foo"
}

function testPromiseClone(): void {
	const promise: Promise<string> = fn()

	arktest.expectError(() => {
		const promiseCopy = deepcopy(promise)
	}, "class std.core.Promise doesn't have default constructor")
}

function main(): int {
	const suite = new arktest.ArkTestsuite("deepcopy() API")

	suite.addTest("object deep copy", testObjectClone)
	suite.addTest("object shallow copy", testObjectShallowCopy)
	suite.addTest("object deep copy depth == 0", testObjectNoCopy)
	suite.addTest("object deep copy depth == 2", testObjectCloneDepth2)
	suite.addTest("object with ref cycle copy", testObjectRefCycleClone)
	suite.addTest("builtin array deep copy", testBuiltinArrayClone)
	suite.addTest("FixedArray<boolean> deep copy", testBoolArrayClone)
	suite.addTest("FixedArray<byte> deep copy", testByteArrayClone)
	suite.addTest("FixedArray<char> deep copy", testCharArrayClone)
	suite.addTest("FixedArray<short> deep copy", testShortArrayClone)
	suite.addTest("FixedArray<int> deep copy", testIntArrayClone)
	suite.addTest("FixedArray<long> deep copy", testLongArrayClone)
	suite.addTest("FixedArray<float> deep copy", testFloatArrayClone)
	suite.addTest("FixedArray<double> deep copy", testDoubleArrayClone)
	suite.addTest("stdlib Array deep copy", testArrayClone)
	suite.addTest("object with lambda field", testObjectWithLambdaClone)
	suite.addTest("stdlib Set deep copy", testSetClone)
	suite.addTest("stdlib Map deep copy", testMapClone)
	suite.addTest("stdlib Date deep copy", testDateClone)
	suite.addTest("WeakRef clone", testWeakRefClone)
	suite.addTest("Promise clone", testPromiseClone)

	return suite.run()
}
