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

export let Identity = function (x) {
	return x;
};
export let Curry = function (x) {
	return {
		toString() {
			return x;
		},
	};
};
export let Throwing = function () {
	return {
		toString() {
			throw 123;
		},
	};
};
export let CurryToString = function (x) {
	return {
		toString() {
			return x.toString();
		},
	};
};
export let IntValue = function () {
	return 42;
};
export let NullVal = function () {
	return null;
};
export let NanVal = function () {
	return NaN;
};
export let UndefinedVal = function () {
	return undefined;
};
export let Optional = function (x) {
	return x ? null : 'optionalVal';
};
export let StringifyValue = function (v) {
	return typeof v + ':' + v;
};
