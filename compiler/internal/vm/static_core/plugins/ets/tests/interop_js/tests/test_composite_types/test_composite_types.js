'use strict';
/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const TEST_STRING = 'This is a test string';
const TEST_INT = 100;
const TEST_BOOLEAN = true;
export const isTTupleTS = (testedVar) => {
	return (
		Array.isArray(testedVar) &&
		testedVar.length === 3 &&
		typeof testedVar[0] === 'string' &&
		typeof testedVar[1] === 'number' &&
		typeof testedVar[2] === 'boolean'
	);
};
const testTuple = [TEST_STRING, TEST_INT, TEST_BOOLEAN];
export const returnTuple = function returnTuple() {
	return testTuple;
};
export const returnStrArray = function returnArray() {
	const testArray = ['One', 'Two', 'Three'];
	return testArray;
};
export const returnNumArray = function returnArrayNum() {
	const testArray = [1, 2, 3];
	return testArray;
};
export const returnRecord = function returnRecord() {
	const testRecord = {
		one: 1,
		two: 2,
		three: 3,
	};
	return testRecord;
};
export const returnInterface = function returnInterface(returnThreeDimensioned) {
	const testTwoDimensioned = {
		width: 100,
		length: 40,
	};
	const testThreeDimensioned = {
		width: 100,
		length: 50,
		height: 25,
	};
	if (returnThreeDimensioned) {
		return testThreeDimensioned;
	}
	return testTwoDimensioned;
};
export class CompositeTypesTestClass {
	constructor() {
		this.returnTuple = returnTuple.bind(this);
		this.returnStrArray = returnStrArray.bind(this);
		this.returnNumArray = returnNumArray.bind(this);
		this.returnRecord = returnRecord.bind(this);
		this.returnInterface = returnInterface.bind(this);
	}
}
export let compositeTypesTestClassInstance = new CompositeTypesTestClass();
const test = () => {
	print([returnTuple, returnStrArray, returnNumArray, returnRecord].map((x) => x()));
};
const testInterface = () => {
	print((0, returnInterface)(true), (0, returnInterface)(false));
};
test();
testInterface();
