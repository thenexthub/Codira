'use strict';
/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
const TEST_LONG_INT = 9007199254740991;
const TEST_INT = 100;
const TEST_BIG_INT = 10000000n;
const TEST_BOOLEAN = true;
const TEST_MAP = new Map([[TEST_STRING.toUpperCase(), TEST_STRING.toLowerCase()]]);
const TEST_SET = new Set([TEST_STRING.toUpperCase(), TEST_STRING.toLowerCase()]);
const TEST_CHANGEABLE_MAP = new Map([['1', 'Test']]);
export const returnIntegerAsAny = function returnInteger() {
	return TEST_INT;
};

export const returnStringAsAny = function returnString() {
	return TEST_STRING;
};

export const returnBigIntegerAsAny = function returnBigInteger() {
	return TEST_BIG_INT;
};

export const returnBooleanAsAny = function returnBoolean() {
	return TEST_BOOLEAN;
};

export const returnUndefinedAsAny = function returnUndefined() {
	return;
};

export const returnNullAsAny = function returnNull() {
	return null;
};

export const returnMapAsAny = function returnMap() {
	return TEST_MAP;
};

export const returnSetAsAny = function returnSet() {
	return TEST_SET;
};

export const returnStrLiteral = function returnStrLiteral() {
	const literalValue = 'Test';
	return literalValue;
};

export const returnIntLiteral = function returnIntLiteral() {
	const literalValue = 1;
	return literalValue;
};

export const returnBoolLiteral = function returnBoolLiteral() {
	const literalValue = false;
	return literalValue;
};

export const returnsBigNLiteral = function returnsBigNLiteral() {
	const literalValue = 100n;
	return literalValue;
};

export const returnMap = function returnMap() {
	return TEST_MAP;
};

export const returnSet = function returnSet() {
	return TEST_SET;
};

export const returnTuple = function returnTuple() {
	return [TEST_STRING, TEST_LONG_INT, TEST_BOOLEAN];
};

export const isTTupleTS = (testedVar) => {
	return (
		Array.isArray(testedVar) &&
		testedVar.length === 3 &&
		typeof testedVar[0] === 'string' &&
		typeof testedVar[1] === 'number' &&
		typeof testedVar[2] === 'boolean'
	);
};

export const returnStringSubsetByRef = function returnStringSubsetByRef() {
	return TEST_STRING;
};

export const returnMapSubsetByRef = function returnMapSubsetByRef() {
	return TEST_CHANGEABLE_MAP;
};

export const returnStringSubsetByValue = function returnString() {
	return TEST_STRING;
};

export const returnLongIntSubsetByValue = function returnInteger() {
	return TEST_LONG_INT;
};

export const returnIntSubsetByValue = function returnInteger() {
	return TEST_INT;
};

export const returnBoolSubsetByValue = function returnBoolean() {
	return TEST_BOOLEAN;
};

export const returnUndefinedSubsetByValue = function returnUndefined() {
	return;
};

export const returnNullSubsetByValue = function returnNull() {
	return null;
};
export const returnUnion = function returnUnion(isStr) {
	if (isStr) {
		return '1000';
	}
	return 1000;
};

export class TestClass {
	constructor(testNumber, testString) {
		this.testNumber = testNumber;
		this.testString = testString;
	}
	testFunction() {
		return this.testNumber;
	}
	static isTestClass(instance) {
		return instance instanceof TestClass;
	}
}

export const returnClass = function returnClass() {
	const testClass = new TestClass(1, 'Test');
	return testClass;
};

export const returnInterface = function returnInterface() {
	const testInterface = {
		testNumber: 100,
		testString: 'Test',
	};
	return testInterface;
};

export class ReturnTestClass {
	constructor() {
		this.returnIntegerAsAny = returnIntegerAsAny.bind(this);
		this.returnStringAsAny = returnStringAsAny.bind(this);
		this.returnBigIntegerAsAny = returnBigIntegerAsAny.bind(this);
		this.returnBooleanAsAny = returnBooleanAsAny.bind(this);
		this.returnUndefinedAsAny = returnUndefinedAsAny.bind(this);
		this.returnNullAsAny = returnNullAsAny.bind(this);
		this.returnMapAsAny = returnMapAsAny.bind(this);
		this.returnSetAsAny = returnSetAsAny.bind(this);
		this.returnStrLiteral = returnStrLiteral.bind(this);
		this.returnIntLiteral = returnIntLiteral.bind(this);
		this.returnBoolLiteral = returnBoolLiteral.bind(this);
		this.returnsBigNLiteral = returnsBigNLiteral.bind(this);
		this.returnMap = returnMap.bind(this);
		this.returnSet = returnSet.bind(this);
		this.returnTuple = returnTuple.bind(this);
		this.returnStringSubsetByRef = returnStringSubsetByRef.bind(this);
		this.returnMapSubsetByRef = returnMapSubsetByRef.bind(this);
		this.returnStringSubsetByValue = returnStringSubsetByValue.bind(this);
		this.returnIntSubsetByValue = returnIntSubsetByValue.bind(this);
		this.returnLongIntSubsetByValue = returnLongIntSubsetByValue.bind(this);
		this.returnBoolSubsetByValue = returnBoolSubsetByValue.bind(this);
		this.returnUndefinedSubsetByValue = returnUndefinedSubsetByValue.bind(this);
		this.returnNullSubsetByValue = returnNullSubsetByValue.bind(this);
		this.returnUnion = returnUnion.bind(this);
		this.returnClass = returnClass.bind(this);
		this.returnInterface = returnInterface.bind(this);
	}
}

export let returnTestClassInstance = new ReturnTestClass();

const testAny = () => {
	print(
		[
			returnIntegerAsAny,
			returnStringAsAny,
			returnBigIntegerAsAny,
			returnBooleanAsAny,
			returnUndefinedAsAny,
			returnNullAsAny,
			returnMapAsAny,
			returnSetAsAny,
		].map((x) => x())
	);
};

const testLiterals = () => {
	print([returnStrLiteral, returnIntLiteral, returnBoolLiteral, returnsBigNLiteral].map((x) => x()));
};

const testExtraSet = () => {
	print([returnMap, returnSet, returnTuple].map((x) => x()));
};

const testSubset = () => {
	print(
		[
			returnStringSubsetByRef,
			returnMapSubsetByRef,
			returnStringSubsetByValue,
			returnIntSubsetByValue,
			returnLongIntSubsetByValue,
			returnBoolSubsetByValue,
			returnUndefinedSubsetByValue,
			returnNullSubsetByValue,
		].map((x) => x())
	);
};

const testUnion = () => {
	print((0, returnUnion)(true), (0, returnUnion)(false));
};

const testClassInterface = () => {
	print([returnClass, returnInterface].map((x) => x()));
};

testAny();
testLiterals();
testExtraSet();
testSubset();
testUnion();
testClassInterface();
