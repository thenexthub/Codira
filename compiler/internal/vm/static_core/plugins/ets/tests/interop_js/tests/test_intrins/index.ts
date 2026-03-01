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

export let nop = function (): void {};

export let identity = function (v): typeof v {
	return v;
};

export let stringifyValue = function (v): string {
	return typeof v + ':' + v;
};

export let stringifyArgs = function (...args): string {
	return args.toString();
};

export let setProtoConstructor = function (v): () => void {
	return Object.getPrototypeOf(v).constructor;
};

export let applyArgs = function (fn, ...args): number {
	return fn(...args);
};

export let throwValue = function (v): void {
	throw v;
};

export let log = function (...args): void {
	print(`${args.join(' ')}`);
};

export let getProp = function (v, p): number {
	return v[p];
};

export let sum = function (a, b): number {
	return a + b;
};

export let makeCar = function (v): void {
	this.color = v;
};

export let makeTestProxy = function (): void {
	Object.defineProperty(this, 'foo', {
		get: function (): never {
			throw Error('get exception');
		},
		set: function (): never {
			throw Error('set exception');
		},
	});
};
