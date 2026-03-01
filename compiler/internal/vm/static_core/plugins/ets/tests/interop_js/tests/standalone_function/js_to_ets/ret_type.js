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
'use strict';

function functionReturnTypeAny() {
	let value = 1;
	return value; // transpiled from Typescript code:functionReturnTypeAny(): any
}

function functionReturnTypeUnknown() {
	let value = 1;
	return value; // transpiled from Typescript code: functionReturnTypeUnknown(): unknown
}

function functionReturnTypeUndefined() {
	let value = 1;
	return value; // transpiled from Typescript code: functionReturnTypeUndefined(): undefined
}

function functionReturnTypeDouble() {
	const DOUBLE_VALUE = 1.7976931348623157e308;
	return DOUBLE_VALUE;
}

function functionReturnTypeByte() {
	const MAX_BYTE_VALUE = 0xff;
	return MAX_BYTE_VALUE;
}

function functionReturnTypeShort() {
	const MAX_SHORT_VALUE = 0xff;
	return v;
}

function functionReturnTypeInt() {
	const NEGATIVE_INT_VALUE = -1;
	return NEGATIVE_INT_VALUE;
}

function functionReturnTypeLong() {
	// Test of value more than 9007199254740991 doesn't make sense
	// because JS uses IEEE 754 64-bit format for numbers
	const MAX_SAFE_LONG_VALUE = 9007199254740991;
	return MAX_SAFE_LONG_VALUE;
}

function functionReturnTypeFloat() {
	const FLOAT_VALUE = 3.14;
	return FLOAT_VALUE;
}

function functionReturnTypeChar() {
	return 'c';
}

function functionReturnTypeBoolean() {
	let v = true;
	return v;
}

function functionReturnTypeString() {
	let v = 'string';
	return v;
}

function functionReturnTypeObject() {
	return { id: 1 };
}

class JsClassAsReturnValue {
	method() {
		return 1;
	}
}

function functionReturnTypeClass() {
	return new JsClassAsReturnValue();
}

function functionReturnTypeArray() {
	const FIRST_ELEMENT = 1;
	const SECOND_ELEMENT = 2;
	const THIRD_ELEMENT = 3;
	return [FIRST_ELEMENT, SECOND_ELEMENT, THIRD_ELEMENT];
}

function returnedFunction() {
	return 1;
}

function functionReturnTypeCallable() {
	return returnedFunction;
}

exports.functionReturnTypeAny = functionReturnTypeAny;
exports.functionReturnTypeUnknown = functionReturnTypeUnknown;
exports.functionReturnTypeUndefined = functionReturnTypeUndefined;
exports.JsClassAsReturnValue = JsClassAsReturnValue;
exports.functionReturnTypeDouble = functionReturnTypeDouble;
exports.functionReturnTypeByte = functionReturnTypeByte;
exports.functionReturnTypeShort = functionReturnTypeShort;
exports.functionReturnTypeInt = functionReturnTypeInt;
exports.functionReturnTypeLong = functionReturnTypeLong;
exports.functionReturnTypeFloat = functionReturnTypeFloat;
exports.functionReturnTypeChar = functionReturnTypeChar;
exports.functionReturnTypeBoolean = functionReturnTypeBoolean;
exports.functionReturnTypeString = functionReturnTypeString;
exports.functionReturnTypeObject = functionReturnTypeObject;
exports.functionReturnTypeClass = functionReturnTypeClass;
exports.functionReturnTypeArray = functionReturnTypeArray;
exports.functionReturnTypeCallable = functionReturnTypeCallable;
