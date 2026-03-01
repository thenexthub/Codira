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
	_value: number;

	constructor(value: number) {
		this._value = value;
	}
}

export const IIFEClass: ClassConstructor<number> = (function (): ClassConstructor<number> {
	return class {
		_value: number;

		constructor(value: number) {
			this._value = value;
		}
	};
})();

export const AnonymousClass = class {
	_value: number;

	constructor(value: number) {
		this._value = value;
	}
};

interface ClassConstructor<T> {
	new (value: T): {};
}
type ClassType = new (...args: {}[]) => {};

export class ParentClass {
	_otherClass: {};

	constructor(otherClass: ClassConstructor<number>) {
		this._otherClass = new otherClass(tsInt);
	}
}

export function createClassWithArgFromTs(arg: ClassConstructor<number>): ParentClass {
	return new ParentClass(arg);
}

export function createMainClassFromTs(): ParentClass {
	return new ParentClass(MainClass);
}

export const mainClassInstance = new ParentClass(MainClass);

export function createAnonymousClassFromTs(): ParentClass {
	return new ParentClass(AnonymousClass);
}

export const anonymousClassInstance = new ParentClass(AnonymousClass);

export function createIIFEClassFromTs(): ParentClass {
	return new ParentClass(IIFEClass);
}

export const iifeClassInstance = new ParentClass(IIFEClass);

export class ChildClass extends ParentClass {
	constructor(otherClass: ClassConstructor<number>) {
		super(otherClass);
	}
}

export function createChildClassWithArgFromTs(arg: ClassConstructor<number>): ChildClass {
	return new ChildClass(arg);
}

export function createChildClassWithMainFromTs(): ChildClass {
	return new ChildClass(MainClass);
}

export const childClassWithMainInstance = new ChildClass(MainClass);

export function createChildClassWithAnonymousFromTs(): ChildClass {
	return new ChildClass(MainClass);
}

export const childClassWithAnonymousInstance = new ChildClass(MainClass);

export function createChildClassWithIIFEFromTs(): ChildClass {
	return new ChildClass(IIFEClass);
}

export const childClassWithIIFEInstance = new ChildClass(IIFEClass);

export const AnonymousClassCreateClass = class {
	_otherClass: MainClass;

	constructor(otherClass: ClassConstructor<number>) {
		this._otherClass = new otherClass(tsInt) as MainClass;
	}
};

export function createAnonymousClassCreateClassWithArgFromTs(arg: ClassConstructor<number>): Object {
	return new AnonymousClassCreateClass(arg);
}

export function createAnonymousClassCreateClassFromTs(): Object {
	return new AnonymousClassCreateClass(MainClass);
}

export function createAnonymousClassCreateIIFEClassFromTs(): Object {
	return new AnonymousClassCreateClass(IIFEClass);
}

export const anonymousClassCreateMainInstance = new AnonymousClassCreateClass(MainClass);

export const anonymousClassCreateIIFEInstance = new AnonymousClassCreateClass(IIFEClass);

export const IIFECreateClassMain = (function (ctor: new (_value: number) => MainClass, value: number): Object {
	return new ctor(value);
})(MainClass, tsInt);

export const IIFECreateClassAnonymous = (function (ctor: new (_value: number) => InstanceType<typeof AnonymousClass>, value: number): Object {
	return new ctor(value);
})(AnonymousClass, tsInt);

export const IIFECreateClass = (function (ctor: new (_value: number) => InstanceType<typeof IIFEClass>, value: number): Object {
	return new ctor(value);
})(IIFEClass, tsInt);

export class MethodClass {
	init(anyClass: ClassConstructor<number>, value: number): Object {
		return new anyClass(value);
	}
}

export function CreateClassFunction(Arg: ClassConstructor<number>, val: number): Object {
	return new Arg(val);
}

export function CreateClassArrowFunction(Arg: ClassConstructor<number>, val: number): Object {
	return new Arg(val);
}

export function checkInstance<T, U>(mainClass: new (...args: {}[]) => T, instance: U): boolean {
	if (typeof mainClass !== 'function' || typeof instance !== 'object' || instance === null) {
		throw new TypeError('must be a class');
	}

	return instance instanceof mainClass;
}
