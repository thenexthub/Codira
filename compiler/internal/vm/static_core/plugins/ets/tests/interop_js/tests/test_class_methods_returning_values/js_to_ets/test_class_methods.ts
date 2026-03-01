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
const TEST_LONG_INT = 9007199254740991;
const TEST_INT = 100;
const TEST_BIG_INT = 10000000n;
const TEST_BOOLEAN = true;
const TEST_MAP = new Map([[TEST_STRING.toUpperCase(), TEST_STRING.toLowerCase()]]);
const TEST_SET = new Set([TEST_STRING.toUpperCase(), TEST_STRING.toLowerCase()]);
const TEST_CHANGEABLE_MAP = new Map([['1', 'Test']]);

type STRING_LITERAL = 'Test';
type INTEGER_LITERAL = 1;
type BOOLEAN_LITERAL = false;
type BIGN_LITERAL = 100n;

export type UnionNumberOrString = number | string;

export type TTuple = [string, number, boolean];

interface TestInterface {
	testNumber: number;
	testString: string;
}

export type TFunctionReturningValue<T> = () => T;

export type TReturnsAny = TFunctionReturningValue<{} | void | null>;
export type TReturnsStrLiteral = TFunctionReturningValue<STRING_LITERAL>;
export type TReturnsIntLiteral = TFunctionReturningValue<INTEGER_LITERAL>;
export type TReturnsBoolLiteral = TFunctionReturningValue<BOOLEAN_LITERAL>;
export type TReturnsBigNLiteral = TFunctionReturningValue<BIGN_LITERAL>;
export type TReturnsMap = TFunctionReturningValue<Map<string, string>>;
export type TReturnsSet = TFunctionReturningValue<Set<string>>;
export type TReturnsTuple = TFunctionReturningValue<TTuple>;
export type TReturnsString = TFunctionReturningValue<string>;
export type TTestClass = TFunctionReturningValue<TestClass>;
export type TTestInterface = TFunctionReturningValue<TestInterface>;

export const returnIntegerAsAny: TReturnsAny = function returnInteger(): {} {
	return TEST_INT;
};

export const returnStringAsAny: TReturnsAny = function returnString(): {} {
	return TEST_STRING;
};

export const returnBigIntegerAsAny: TReturnsAny = function returnBigInteger(): {} {
	return TEST_BIG_INT;
};

export const returnBooleanAsAny: TReturnsAny = function returnBoolean(): {} {
	return TEST_BOOLEAN;
};

export const returnUndefinedAsAny: TReturnsAny = function returnUndefined(): {} | void {
	return undefined;
};

export const returnNullAsAny: TReturnsAny = function returnNull(): {} | void | null {
	return null;
};

export const returnMapAsAny: TReturnsAny = function returnMap(): {} {
	return TEST_MAP;
};

export const returnSetAsAny: TReturnsAny = function returnSet(): {} {
	return TEST_SET;
};

export const returnStrLiteral: TReturnsStrLiteral = function returnStrLiteral(): STRING_LITERAL {
	const literalValue: STRING_LITERAL = 'Test';
	return literalValue;
};

export const returnIntLiteral: TReturnsIntLiteral = function returnIntLiteral(): INTEGER_LITERAL {
	const literalValue: INTEGER_LITERAL = 1;
	return literalValue;
};

export const returnBoolLiteral: TReturnsBoolLiteral = function returnBoolLiteral(): BOOLEAN_LITERAL {
	const literalValue: BOOLEAN_LITERAL = false;
	return literalValue;
};

export const returnsBigNLiteral: TReturnsBigNLiteral = function returnsBigNLiteral(): BIGN_LITERAL {
	const literalValue: BIGN_LITERAL = 100n;
	return literalValue;
};

export const returnMap: TReturnsMap = function returnMap() {
	return TEST_MAP;
};

export const returnSet: TReturnsSet = function returnSet() {
	return TEST_SET;
};

export const returnTuple: TReturnsTuple = function returnTuple() {
	return [TEST_STRING, TEST_LONG_INT, TEST_BOOLEAN];
};

export const isTTupleTS = (testedVar: unknown): testedVar is TTuple => {
	return (
		Array.isArray((<Object>testedVar) as Object) &&
		(<Array<unknown>>testedVar).length === 3 &&
		typeof (<Array<unknown>>testedVar)[0] === 'string' &&
		typeof (<Array<unknown>>testedVar)[1] === 'number' &&
		typeof (<Array<unknown>>testedVar)[2] === 'boolean'
	);
};

export const returnStringSubsetByRef: TReturnsString = function returnStringSubsetByRef(): Object {
	return TEST_STRING;
};

export const returnMapSubsetByRef: TReturnsMap = function returnMapSubsetByRef(): Object {
	return TEST_CHANGEABLE_MAP;
};

export const returnStringSubsetByValue = function returnString(): Object {
	return TEST_STRING;
};

export const returnLongIntSubsetByValue = function returnInteger(): Object {
	return TEST_LONG_INT;
};

export const returnIntSubsetByValue = function returnInteger(): Object {
	return TEST_INT;
};

export const returnBoolSubsetByValue = function returnBoolean(): Object {
	return TEST_BOOLEAN;
};

export const returnUndefinedSubsetByValue = function returnUndefined(): Object {
	return;
};

export const returnNullSubsetByValue = function returnNull(): Object {
	return null;
};

export const returnUnion = function returnUnion(isStr: boolean): UnionNumberOrString {
	if (isStr) {
		return '1000';
	}
	return 1000;
};

export class TestClass {
	testNumber: number;
	testString: string;

	constructor(testNumber: number, testString: string) {
		this.testNumber = testNumber;
		this.testString = testString;
	}

	testFunction(): number {
		return this.testNumber;
	}

	public static isTestClass(instance: TestClass): boolean {
		return instance instanceof TestClass;
	}
}

export const returnClass: TTestClass = function returnClass() {
	const testClass: TestClass = new TestClass(1, 'Test');
	return testClass;
};

export const returnInterface: TTestInterface = function returnInterface() {
	const testInterface: TestInterface = {
		testNumber: 100,
		testString: 'Test',
	};
	return testInterface;
};

export class ReturnTestClass {
	public returnIntegerAsAny = returnIntegerAsAny.bind(this);
	public returnStringAsAny = returnStringAsAny.bind(this);
	public returnBigIntegerAsAny = returnBigIntegerAsAny.bind(this);
	public returnBooleanAsAny = returnBooleanAsAny.bind(this);
	public returnUndefinedAsAny = returnUndefinedAsAny.bind(this);
	public returnNullAsAny = returnNullAsAny.bind(this);
	public returnMapAsAny = returnMapAsAny.bind(this);
	public returnSetAsAny = returnSetAsAny.bind(this);

	public returnStrLiteral = returnStrLiteral.bind(this);
	public returnIntLiteral = returnIntLiteral.bind(this);
	public returnBoolLiteral = returnBoolLiteral.bind(this);
	public returnsBigNLiteral = returnsBigNLiteral.bind(this);

	public returnMap = returnMap.bind(this);
	public returnSet = returnSet.bind(this);
	public returnTuple = returnTuple.bind(this);

	public returnStringSubsetByRef = returnStringSubsetByRef.bind(this);
	public returnMapSubsetByRef = returnMapSubsetByRef.bind(this);
	public returnStringSubsetByValue = returnStringSubsetByValue.bind(this);
	public returnIntSubsetByValue = returnIntSubsetByValue.bind(this);
	public returnLongIntSubsetByValue = returnLongIntSubsetByValue.bind(this);
	public returnBoolSubsetByValue = returnBoolSubsetByValue.bind(this);
	public returnUndefinedSubsetByValue = returnUndefinedSubsetByValue.bind(this);
	public returnNullSubsetByValue = returnNullSubsetByValue.bind(this);

	public returnUnion = returnUnion.bind(this);
	public returnClass = returnClass.bind(this);
	public returnInterface = returnInterface.bind(this);
}

export type TReturnTestClass = ReturnTestClass;
export const returnTestClassInstance = new ReturnTestClass();

const testAny = (): void => {
	console.log(
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

const testLiterals = (): void => {
	console.log([returnStrLiteral, returnIntLiteral, returnBoolLiteral, returnsBigNLiteral].map((x) => x()));
};

const testExtraSet = (): void => {
	console.log([returnMap, returnSet, returnTuple].map((x) => x()));
};

const testSubset = (): void => {
	console.log(
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

const testUnion = (): void => {
	console.log(returnUnion(true), returnUnion(false));
};

const testClassInterface = (): void => {
	console.log([returnClass, returnInterface].map((x) => x()));
};

testAny();
testLiterals();
testExtraSet();
testSubset();
testUnion();
testClassInterface();
