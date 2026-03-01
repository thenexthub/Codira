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

type Union = number | boolean;

export class UnionSetter {
	private _value?: Union;

	set value(arg: Union) {
		this._value = arg;
	}

	get value(): Union | undefined {
		return this._value;
	}
}

interface ISetter {
	_value?: string;

	set value(arg: string);

	get value(): string | undefined;
}

export class InterfaceSetter implements ISetter {
	_value?: string;

	set value(arg: string) {
		this._value = arg;
	}

	get value(): string | undefined {
		return this._value;
	}
}

export class BaseClass {
	private _value?: string;

	set value(arg: string) {
		this._value = arg;
	}

	get value(): string | undefined {
		return this._value;
	}
}

export class SubsetRefSet extends BaseClass {}

export const tsTestString = 'ts_test_string';

class ValueSetter {
	protected _value = tsTestString;
}

export class SubsetValueSet extends ValueSetter {
	constructor() {
		super();
	}

	set value(arg: string) {
		this._value = arg;
	}

	get value(): string {
		return this._value;
	}
}

export class SetterAnyType<T> {
	private _value?: T;

	set value(arg: T) {
		this._value = arg;
	}

	get value(): T | undefined {
		return this._value;
	}
}

type TupleT = [number, string];

export class TupleSet {
	private _value?: TupleT;

	public set value(arg: TupleT) {
		this._value = arg;
	}

	public get value(): TupleT | undefined {
		return this._value;
	}
}

abstract class AbstractSetter {
	protected _value?: string;

	abstract set value(arg: string);

	abstract get value(): string | undefined;
}

export class AbstractClass extends AbstractSetter {
	override set value(arg: string) {
		this._value = arg;
	}

	override get value(): string | undefined {
		return this._value;
	}
}

export const AbstractClassObject = new AbstractClass();
export const UnionSetterObject = new UnionSetter();
export const InterfaceSetterObject = new InterfaceSetter();
export const TupleSetObject = new TupleSet();
export const SetterAnyTypeObject = new SetterAnyType();
export const BaseClassObject = new BaseClass();
export const SubsetRefSetObject = new SubsetRefSet();
export const SubsetValueSetObject = new SubsetValueSet();
