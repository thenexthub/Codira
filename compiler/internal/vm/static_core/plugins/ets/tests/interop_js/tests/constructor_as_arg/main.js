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
export const tsInt = 1;
export class MainClass {
	constructor(value) {
		this._value = value;
	}
};
export let IIFEClass = (function () {
	return class {
		constructor(value) {
			this._value = value;
		}
	};
})();
export let AnonymousClass = class {
	constructor(value) {
		this._value = value;
	}
};
export class ParentClass {
	constructor(otherClass) {
		this._otherClass = new otherClass(tsInt);
	}
};
export function createClassWithArgFromTs(arg) {
	return new ParentClass(arg);
};
export function createMainClassFromTs() {
	return new ParentClass(MainClass);
};
export let mainClassInstance = new ParentClass(MainClass);
export function createAnonymousClassFromTs() {
	return new ParentClass(AnonymousClass);
};
export let anonymousClassInstance = new ParentClass(AnonymousClass);
export function createIIFEClassFromTs() {
	return new ParentClass(IIFEClass);
};
export let iifeClassInstance = new ParentClass(IIFEClass);
export class ChildClass extends ParentClass {
	constructor(otherClass) {
		super(otherClass);
	}
};
export function createChildClassWithArgFromTs(arg) {
	return new ChildClass(arg);
};
export function createChildClassWithMainFromTs() {
	return new ChildClass(MainClass);
};
export let childClassWithMainInstance = new ChildClass(MainClass);
export function createChildClassWithAnonymousFromTs() {
	return new ChildClass(MainClass);
};
export let childClassWithAnonymousInstance = new ChildClass(MainClass);
export function createChildClassWithIIFEFromTs() {
	return new ChildClass(IIFEClass);
};
export let childClassWithIIFEInstance = new ChildClass(IIFEClass);
export let AnonymousClassCreateClass = class {
	constructor(otherClass) {
		this._otherClass = new otherClass(tsInt);
	}
};
export function createAnonymousClassCreateClassWithArgFromTs(arg) {
	return new AnonymousClassCreateClass(arg);
};
export function createAnonymousClassCreateClassFromTs() {
	return new AnonymousClassCreateClass(MainClass);
};
export function createAnonymousClassCreateIIFEClassFromTs() {
	return new AnonymousClassCreateClass(IIFEClass);
};
export let anonymousClassCreateMainInstance = new AnonymousClassCreateClass(MainClass);
export let anonymousClassCreateIIFEInstance = new AnonymousClassCreateClass(IIFEClass);
export let IIFECreateClassMain = (function (ctor, value) {
	return new ctor(value);
})(MainClass, tsInt);
export let IIFECreateClassAnonymous = (function (ctor, value) {
	return new ctor(value);
})(AnonymousClass, tsInt);
export let IIFECreateClass = (function (ctor, value) {
	return new ctor(value);
})(IIFEClass, tsInt);
export class MethodClass {
	init(anyClass, value) {
		return new anyClass(value);
	}
};
export function CreateClassFunction(Arg, val) {
	return new Arg(val);
};
export function CreateClassArrowFunction(Arg, val) {
	return new Arg(val);
};
export function checkInstance(MainClass, instance) {
	if (typeof MainClass !== 'function' || typeof instance !== 'object' || instance === null) {
		throw new TypeError('must be a class');
	}
	return instance instanceof MainClass;
};
