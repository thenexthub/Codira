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

export function cons(value, tail) {
	return { value: value, tail: tail };
}

export function car(node) {
	return node.value;
}

export function cdr(node) {
	return node.tail;
}

export function sum(a, b) {
	return a + b;
}

export class TreeNode {
	value;
	left;
	right;

	constructor(value, left, right) {
		this.value = value;
		this.left = left;
		this.right = right;
	}

	sum() {
		let res = this.value;
		if (this.left) {
			res += this.left.sum();
		}
		if (this.right) {
			res += this.right.sum();
		}
		return res;
	}
}

export function makeDynObject(x) {
	this.v0 = { value: x };
}

export function doNothing() {}

export function makeSwappable(obj) {
	obj.swap = function () {
		let tmp = this.first;
		this.first = this.second;
		this.second = tmp;
	};
}

export class StaticClass {
	static staticProperty = 10;
	static staticMethod() {
		return this.staticProperty + 100;
	}
}

export function extractSquaredInt(obj) {
	let x = obj.intValue;
	return x * x;
}

function MakeObjectWithPrototype() {
	this.overriddenValue = 4;
	this.overriddenFunction = function () {
		return 'overridden';
	};
}

let o1 = new MakeObjectWithPrototype();

MakeObjectWithPrototype.prototype.overriddenValue = -1;
MakeObjectWithPrototype.prototype.overriddenFunction = function () {
	return 'should be overridden';
};
MakeObjectWithPrototype.prototype.prototypeValue = 5;
MakeObjectWithPrototype.prototype.prototypeFunction = function () {
	return 'prototype function';
};

export let dynStorage = {
	str: 'abcd',
	dbl: 1.9,
	integer: 6,
	boolTrue: true,
	boolFalse: false,
	verify: function () {
		const isStringValid = Number(this.str === 'dcba');
		const isFloatValid = Number(Math.abs(this.dbl - 2.4) < Number.EPSILON);
		const isIntegerValid = Number(this.integer === 31);
		const isBooleanValid = Number(this.boolTrue === false);
		return isStringValid + isFloatValid + isIntegerValid + isBooleanValid;
	},
};

export let ObjectWithPrototype = MakeObjectWithPrototype;
export let vundefined = undefined;
