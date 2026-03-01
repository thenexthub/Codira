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

export class PublicGetterClass {
	_value = tsString;

	get value(): string {
		return this._value;
	}
}

export function createPublicGetterClassFromTs(): PublicGetterClass {
	return new PublicGetterClass();
}

export const publicGetterInstanceClass = new PublicGetterClass();

export class ProtectedGetterOrigenClass {
	protected _value = tsString;

	protected get value(): string {
		return this._value;
	}
}

export function createProtectedGetterOrigenClassFromTs(): ProtectedGetterOrigenClass {
	return new ProtectedGetterOrigenClass();
}

export const protectedGetterOrigenInstanceClass = new ProtectedGetterOrigenClass();

export class ProtectedGetterInheritanceClass extends ProtectedGetterOrigenClass {}

export function createProtectedGetterInheritanceClassFromTs(): ProtectedGetterInheritanceClass {
	return new ProtectedGetterInheritanceClass();
}

export const protectedGetterInstanceInheritanceClass = new ProtectedGetterInheritanceClass();

export class PrivateGetterClass {
	private _value = tsString;

	private get value(): string {
		return this._value;
	}
}

export function createPrivateGetterClassFromTs(): PrivateGetterClass {
	return new PrivateGetterClass();
}

export const privateGetterInstanceClass = new PrivateGetterClass();

export type UnionType = number | string;

export class UnionTypeClass {
	private _value: UnionType;

	constructor(value: UnionType) {
		this._value = value;
	}

	public get value(): UnionType {
		return this._value;
	}
}

export function createUnionTypeGetterClassFromTs(arg: UnionType): UnionTypeClass {
	return new UnionTypeClass(arg);
}

export const unionTypeGetterInstanceClassInt = new UnionTypeClass(tsNumber);
export const unionTypeGetterInstanceClassString = new UnionTypeClass(tsString);

export type LiteralValue = 1 | 'string';

export class LiteralClass {
	private _value?: LiteralValue;

	constructor(value: LiteralValue) {
		this._value = value;
	}

	public get value(): LiteralValue | undefined {
		return this._value;
	}
}

export function createLiteralTypeGetterClassFromTs(arg: LiteralValue): LiteralClass {
	return new LiteralClass(arg);
}

export const literalTypeGetterInstanceClassInt = new LiteralClass(tsNumber);
export const literalTypeGetterInstanceClassString = new LiteralClass(tsString);

export type TupleType = [number, string];

export class TupleTypeClass {
	private _value: TupleType;

	constructor(value: TupleType) {
		this._value = value;
	}

	public get value(): TupleType {
		return this._value;
	}
}

export function createTupleTypeGetterClassFromTs(arg: TupleType): TupleTypeClass {
	return new TupleTypeClass(arg);
}

export const tupleTypeGetterInstanceClass = new TupleTypeClass([tsNumber, tsString]);

export class AnyTypeClass<T> {
	public _value?: T;

	public get value(): T | undefined {
		return this._value;
	}
}

export function createAnyTypeGetterClassFromTs(): anyTypeClass<{}> {
	return new AnyTypeClass();
}

export const anyTypeGetterInstanceClass = new AnyTypeClass();
export const anyTypeExplicitGetterInstanceClass = new AnyTypeClass<string>();

export class SubsetByRef {
	private RefClass: PublicGetterClass;

	constructor() {
		this.RefClass = new PublicGetterClass();
	}

	public get value(): string {
		return this.RefClass.value;
	}
}

export function createSubsetByRefGetterClassFromTs(): SubsetByRef {
	return new SubsetByRef();
}

export const subsetByRefInstanceClass = new SubsetByRef();

export class SubsetByValueClass {
	public _value: string;

	constructor(value: string) {
		this._value = value;
	}

	get value(): string {
		return this._value;
	}
}

const GClass = new PublicGetterClass();

export function createSubsetByValueGetterClassFromTs(): SubsetByValueClass {
	return new SubsetByValueClass(GClass.value);
}

export const subsetByValueInstanceClass = new SubsetByValueClass(GClass.value);
