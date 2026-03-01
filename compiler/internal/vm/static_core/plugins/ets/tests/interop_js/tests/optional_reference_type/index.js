'use strict';
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
export function fnWithAnyParams(arr) {
	return arr ? arr[0] : 'Argument not found';
}

export function fnWithLiteralParam(arr) {
	return arr ? arr[0] : 'Argument not found';
}

export function fnWithExtraSetParam(arr) {
	return arr ? arr[0] : 'Argument not found';
}

export function fnWithSubsetPick(obj) {
	return obj.address ? obj.address.city : 'Address not found';
}

export function fnWithSubsetOmit(obj) {
	return obj.address ? obj.address.city : 'Address not found';
}

export function fnWithSubsetPartial(obj) {
	return obj.address ? obj.address.city : 'Address not found';
}

export function fnWithUnionParam(obj) {
	if (obj) {
		if (Array.isArray(obj)) {
			return 'This is an array';
		}
		return 'This is an object';
	}
	return 'Argument not found';
}

export let TestUserClass = /** @class */ (function () {
	function testUserClass(id, name) {
		this.id = id;
		this.name = name;
	}
	return testUserClass;
})();

export function fnWithUserClass(obj) {
	return obj ? obj.name : 'Class was not passed';
}

export function fnWithUserInterface(obj) {
	return obj ? obj.name : 'Object was not passed';
}

export function fnWithAnyParamObject(obj) {
	return obj.arr ? obj.arr[0] : obj.id;
}
