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

export const tsString = 'string';
export const tsNumber = 1;
type UnionType = string | number;

type LiteralType = 1 | 'string';

export class LiteralClass<T extends LiteralType> {
	private _value: T;

	constructor(value: T) {
		this._value = value;
	}

	set(arg: T): void {
		this._value = arg;
	}

	get(): T {
		return this._value;
	}
}

export class UnionClass<T extends UnionType> {
	private _value: T;

	constructor(value: T) {
		this._value = value;
	}

	set(arg: T): void {
		this._value = arg;
	}

	get(): T {
		return this._value;
	}
}

interface GInterface<T> {
	value: T;
}

export class InterfaceClass<T> implements GInterface<T> {
	public value: T;

	constructor(value: T) {
		this.value = value;
	}

	set(arg: T): void {
		this.value = arg;
	}

	get(): T {
		return this.value;
	}
}

export abstract class GAbstract<T> {
	private _value: T;

	constructor(value: T) {
		this._value = value;
	}

	set(arg: T): void {
		this._value = arg;
	}

	get(): T {
		return this._value;
	}
}

export class AbstractClass<T> extends GAbstract<T> {
	constructor(value: T) {
		super(value);
	}
}

export class GClass<T> {
	public content: T;

	constructor(content: T) {
		this.content = content;
	}

	public get(): T {
		return this.content;
	}
}

export function genericFunction<T>(arg: T): T {
	return arg;
}

export function tupleDeclaredType<T, U>(items: [T, U]): [T, U] {
	return items;
}

interface Data {
	data: string;
}

export function genericSubsetRef<T extends Data>(items: T): T {
	return items;
}

type TGenericFn<T> = () => T;

export const explicitlyDeclaredType: TGenericFn<string> = () => {
	return tsString;
};

export const literalClass = new LiteralClass(tsString);
export const unionClass = new UnionClass<string>(tsString);
export const interfaceClass = new InterfaceClass<string>(tsString);
export const abstractClass = new AbstractClass<string>(tsString);
