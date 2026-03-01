/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

export type TTuple = [string, number, boolean];
export type TRecord = Record<string, number>;

export type TFunctionReturningValue<T> = () => T;

export type TReturnsTuple = TFunctionReturningValue<TTuple>;
export type TReturnsStrArray = TFunctionReturningValue<string[]>;
export type TReturnsNumArray = TFunctionReturningValue<number[]>;
export type TReturnsRecord = TFunctionReturningValue<TRecord>;

type AnyType = {};

export const isTTupleTS = (testedVar: AnyType): testedVar is TTuple => {
	return (
		Array.isArray(<AnyType>testedVar) &&
		testedVar.length === 3 &&
		typeof testedVar[0] === 'string' &&
		typeof testedVar[1] === 'number' &&
		typeof testedVar[2] === 'boolean'
	);
};

const testTuple: TTuple = [TEST_STRING, TEST_INT, TEST_BOOLEAN];
export const returnTuple: TReturnsTuple = function returnTuple(): Object {
	return testTuple;
};

export const returnStrArray: TReturnsStrArray = function returnArray(): Object {
	const testArray: string[] = ['One', 'Two', 'Three'];
	return testArray;
};

export const returnNumArray: TReturnsNumArray = function returnArrayNum(): Object {
	const testArray: number[] = [1, 2, 3];
	return testArray;
};

export const returnRecord: TReturnsRecord = function returnRecord(): Object {
	const testRecord: TRecord = {
		one: 1,
		two: 2,
		three: 3,
	};
	return testRecord;
};

interface Shape {
	width: number;
}

interface TwoDimensioned extends Shape {
	length: number;
}

interface ThreeDimensioned extends Shape {
	length: number;
	height: number;
}

export const returnInterface = function returnInterface(returnThreeDimensioned: boolean): TwoDimensioned | ThreeDimensioned {
	const testTwoDimensioned: TwoDimensioned = {
		width: 100,
		length: 40,
	};
	const testThreeDimensioned: ThreeDimensioned = {
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
	public returnTuple = returnTuple.bind(this);
	public returnStrArray = returnStrArray.bind(this);
	public returnNumArray = returnNumArray.bind(this);
	public returnRecord = returnRecord.bind(this);
	public returnInterface = returnInterface.bind(this);
}

export type TCompositeTypesTestClass = CompositeTypesTestClass;
export const compositeTypesTestClassInstance = new CompositeTypesTestClass();

const test = (): void => {
	console.log([returnTuple, returnStrArray, returnNumArray, returnRecord].map((x) => x()));
};

const testInterface = (): void => {
	console.log(returnInterface(true), returnInterface(false));
};

test();
testInterface();
