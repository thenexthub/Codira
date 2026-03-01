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

export const tsNumber = 1;
export const tsString = 'string';
interface AnyTypeMethod<T> {
	get(a: T): T;
}

export class AnyTypeMethodClass<T> implements AnyTypeMethod<T> {
	get(a: T): T {
		return a;
	}
}

export function createInterfaceClassAnyTypeMethod(): AnyTypeMethodClass<unknown> {
	return new AnyTypeMethodClass<unknown>();
}

type UnionType = string | number;

interface UnionTypeMethod {
	get(a: UnionType): UnionType;
}

export class UnionTypeMethodClass implements UnionTypeMethod {
	get(a: UnionType): UnionType {
		return a;
	}
}

export function createInterfaceClassUnionTypeMethod(): AnyTypeMethodClass<number> {
	return new AnyTypeMethodClass<number>();
}

interface ArgInterface {
	get(): number;
}

export function subsetByRefInterface(obj: ArgInterface): number {
	return obj.get();
}

class UserClass {
	public value = 1;
}

interface SubsetByValue {
	get(): UserClass;
}

export class SubsetByValueClass implements SubsetByValue {
	get(): UserClass {
		return new UserClass();
	}
}

export function createSubsetByValueClassFromTs(): SubsetByValueClass {
	return new SubsetByValueClass();
}

interface OptionalMethod {
	getNum?(): number;
	getStr(): string;
}

export class WithOptionalMethodClass implements OptionalMethod {
	getNum(): number {
		return tsNumber;
	}

	getStr(): string {
		return tsString;
	}
}

export class WithoutOptionalMethodClass implements OptionalMethod {
	getStr(): string {
		return tsString;
	}
}

export function createClassWithOptionalMethod(): WithOptionalMethodClass {
	return new WithOptionalMethodClass();
}

export function createClassWithoutOptionalMethod(): WithoutOptionalMethodClass {
	return new WithoutOptionalMethodClass();
}

interface OptionalArgResult {
	with: WithOptionalMethodClass;
	without?: WithoutOptionalMethodClass;
}

export function optionalArg(arg: WithOptionalMethodClass, optional?: WithoutOptionalMethodClass): OptionalArgResult {
	if (optional) {
		return { with: arg, without: optional };
	}

	return { with: arg };
}

type ArrayArg = (WithOptionalMethodClass | WithoutOptionalMethodClass)[];

export function optionalArgArray(...arg: ArrayArg): OptionalArgResult {
	const withOptional = arg[0] as WithOptionalMethodClass;
	const withoutOptional = arg[1] as WithoutOptionalMethodClass;

	if (withoutOptional) {
		return { with: withOptional, without: withoutOptional };
	}

	return { with: withOptional };
}

interface TupleType {
	get(arg: [number, string]): [number, string];
}

export class TupleTypeMethodClass implements TupleType {
	get(arg: [number, string]): [number, string] {
		return arg;
	}
}

export function createInterfaceClassTupleTypeMethodFromTs(): TupleTypeMethodClass {
	return new TupleTypeMethodClass();
}

export const unionTypeMethodInstanceClass = new UnionTypeMethodClass();
export const subsetByValueInstanceClass = new SubsetByValueClass();
export const tupleInstanceClass = new TupleTypeMethodClass();
export const withoutOptionalMethodInstanceClass = new WithoutOptionalMethodClass();
export const withOptionalMethodInstanceClass = new WithOptionalMethodClass();
