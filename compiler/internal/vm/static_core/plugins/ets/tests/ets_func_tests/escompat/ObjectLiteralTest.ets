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

class Obj {
    b: number = 4
    c: number = 5
    s: string = 'somestringTest'
}

function testObjectAssignKeys(): void {
	const obj = new Obj()
	const newObj = Object.assign({}, obj)

	const objKeys = Object.keys(obj)
	const newObjKeys = Object.keys(newObj)
 	arktest.assertEQ(objKeys.length,  newObjKeys.length, "unexpected keys number")

	for (let i = 0; i < objKeys.length; i++) {
	 	arktest.assertEQ(objKeys[i],  newObjKeys[i], "unexpected property name: " + newObjKeys[i])

		const propName = objKeys[i]
		const newObjVal = reflect.Value.of(newObj) as ClassValue
		const propVal = newObj[propName]

		arktest.assertEQ((reflect.Value.of(obj) as ClassValue).getFieldByName(propName).getData(),
					      propVal, `unexpected '${propName}' property value: ${propVal}`)
	}
}

function objEntriesComparator(l: [String, Any], r: [String, Any]): number {
	const lname = l[0]
	const rname = r[0]

	if (lname < rname) {
		return -1
	} else if (lname == rname) {
		return 0
	} else {
		return 1
	}
}

function testObjectAssignEntries(): void {
	const obj = new Obj()
	const newObj = Object.assign({}, obj)

	const objEntries = Object.entries(obj)
	const newObjEntries = Object.entries(newObj)
 arktest.assertEQ(objEntries.length,  newObjEntries.length, "unexpected number of object entries")

	for (let i = 0; i < objEntries.length; i++) {
		const objEntryName = objEntries[i]![0]
		const objEntryVal = objEntries[i]![1]

		const newObjEntryName = newObjEntries[i]![0]
		const newObjEntryVal = newObjEntries[i]![1]

	 arktest.assertEQ(objEntryName,  newObjEntryName, "unexpected entry name: " + newObjEntryName)
	 arktest.assertEQ(objEntryVal,  newObjEntryVal, `unexpected '${objEntryName}' property value: ${newObjEntryVal}`)
	}
}

function testObjectAssignValues(): void {
	const obj = new Obj()
	const newObj = Object.assign({}, obj)

	const objVals = Object.values(obj)
	const newObjVals = Object.values(newObj)
 arktest.assertEQ(objVals.length,  newObjVals.length, "unexpected values number")

	for (let i = 0; i < objVals.length; i++) {
		const objVal = objVals[i]
		const newObjVal = newObjVals[i]

	 arktest.assertEQ(objVal,  newObjVal, `unexpected object property value: ${newObjVal} at index [${i}]`)
	}
}

function testObjectAssignGetOwnPropNames(): void {
	const obj = new Obj()
	const  newObj = Object.assign({}, obj)

	const objProps = Object.getOwnPropertyNames(obj)
	const newObjProps = Object.getOwnPropertyNames(newObj)
 arktest.assertEQ(objProps.length,  newObjProps.length, "unexpected number of properties")

	for (let i = 0; i < objProps.length; i++) {
		const objProp = objProps[i]
		const newObjProp = newObjProps[i]

	 arktest.assertEQ(objProp,  newObjProp, "unexpected property name: " + newObjProp)
	}
}

function main(): int {
	const suite = new arktest.ArkTestsuite("Object literal of std.core.Record type tests")

	suite.addTest("Object.assign(Record) result keys", testObjectAssignKeys)
	suite.addTest("Object.assign(Record) result entries", testObjectAssignEntries)
	suite.addTest("Object.assign(Record) result values", testObjectAssignValues)
	suite.addTest("Object.assign(Record) result property names", testObjectAssignGetOwnPropNames)

	return suite.run()
}
