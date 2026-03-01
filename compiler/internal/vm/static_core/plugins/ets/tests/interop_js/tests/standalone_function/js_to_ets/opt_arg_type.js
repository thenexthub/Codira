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

function returnedFunction() {
	return 1;
}
function functionArgTypeDoubleOpt(arg = 1.7976931348623157e308) {
	return arg;
}

function functionArgTypeByteOpt(arg = -128) {
	return arg;
}

function functionArgTypeShortOpt(arg = -32768) {
	return arg;
}

function functionArgTypeIntOpt(arg = -2147483648) {
	return arg;
}

function functionArgTypeLongOpt(arg = 9007199254740991) {
	return arg;
}

function functionArgTypeFloatOpt(arg = 3.14) {
	return arg;
}

function functionArgTypeCharOpt(arg = 'c') {
	return arg;
}

function functionArgTypeBooleanOpt(arg = true) {
	return arg;
}

function functionArgTypeStringOpt(arg = 'test') {
	return arg;
}

function functionArgTypeObjectOpt(arg = { id: 1 }) {
	return arg.id;
}

function functionArgTypeClassOpt(arg = { method: () => 1 }) {
	return arg.method();
}

const FIRST_ELEMENT = 1;
const SECOND_ELEMENT = 2;
const THIRD_ELEMENT = 3;

function functionArgTypeArrayOpt(arg = [FIRST_ELEMENT, SECOND_ELEMENT, THIRD_ELEMENT]) {
	return arg[0];
}

function functionArgTypeTupleOpt(arg = [1, 'test']) {
	return arg[0];
}

function functionArgTypeCallableOpt(arg = returnedFunction) {
	return arg();
}

exports.functionArgTypeDoubleOpt = functionArgTypeDoubleOpt;
exports.functionArgTypeByteOpt = functionArgTypeByteOpt;
exports.functionArgTypeShortOpt = functionArgTypeShortOpt;
exports.functionArgTypeIntOpt = functionArgTypeIntOpt;
exports.functionArgTypeLongOpt = functionArgTypeLongOpt;
exports.functionArgTypeFloatOpt = functionArgTypeFloatOpt;
exports.functionArgTypeCharOpt = functionArgTypeCharOpt;
exports.functionArgTypeBooleanOpt = functionArgTypeBooleanOpt;
exports.functionArgTypeStringOpt = functionArgTypeStringOpt;
exports.functionArgTypeObjectOpt = functionArgTypeObjectOpt;
exports.functionArgTypeClassOpt = functionArgTypeClassOpt;
exports.functionArgTypeArrayOpt = functionArgTypeArrayOpt;
exports.functionArgTypeTupleOpt = functionArgTypeTupleOpt;
exports.functionArgTypeCallableOpt = functionArgTypeCallableOpt;
