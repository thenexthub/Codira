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

export function functionArgTypeConflictArray() {
	const SAMPLE_INT = 123;
	const SAMPLE_STRING = 'foo';

	const array = Array.of(SAMPLE_INT, SAMPLE_STRING);
	return array;
}

export function functionArgTypeConflictArrayBuffer() {
	const BUFFER_SIZE = 8;
	const VALUE_AS_INT32 = 123;

	const buffer = new ArrayBuffer(BUFFER_SIZE);
	const view = new Int32Array(buffer);

	view.set([VALUE_AS_INT32], 0);

	return buffer;
}

export function functionArgTypeConflictBoolean() {
	return new Boolean(true);
}

export function functionArgTypeConflictDataView() {
	const BUFFER_SIZE = 8;
	const VALUE_AS_INT32 = 123;

	const buffer = new ArrayBuffer(BUFFER_SIZE);
	const view = new DataView(buffer);
	view.setInt32(0, VALUE_AS_INT32);

	return view;
}

export function functionArgTypeConflictDate() {
	return new Date(1);
}

export function functionArgTypeConflictError() {
	const SAMPLE_STRING = 'foo';

	return new Error(SAMPLE_STRING);
}

export function functionArgTypeConflictMap() {
	const dataMap = new Map();
	const KEY = 'foo';
	const VALUE = 'bar';

	dataMap.set(KEY, VALUE);

	return dataMap;
}

export function functionArgTypeConflictObject() {
	return new Object();
}

export function functionArgTypeConflictString() {
	const SAMPLE_STRING = 'foo';
	return SAMPLE_STRING;
}
