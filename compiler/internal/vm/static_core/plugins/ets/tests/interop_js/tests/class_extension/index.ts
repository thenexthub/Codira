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

//@ts-ignore -- to avoid @types/node missing
import { EventEmitter } from 'stream';
//@ts-ignore -- to avoid @types/node missing
const etsVm = require('lib/module/ets_interop_js_napi');

type ClassType = new (...args: {}[]) => {};

const ETS_MODULE_NAME: string = 'interop_class_extension';
const makePrefix = (packageName: string): string => 'L' + packageName.replace('.', '/') + '/';

const getClass = (name: string, packageName?: string): ClassType => etsVm.getClass(makePrefix(packageName ?? ETS_MODULE_NAME) + name + ';');

export const DEFAULT_STRING_VALUE = 'Panda';
//@ts-ignore -- to avoid @types/node missing
export const DEFAULT_NUMERIC_VALUE = Math.round(Math.random() * Number!.MAX_SAFE_INTEGER);

type AnyType = {};

export const helperIsJSInstanceOf = (obj: AnyType, classObj: Function): boolean => {
	return obj instanceof classObj;
};

export class TestUserClass {
	private _privateValue: string = DEFAULT_STRING_VALUE;
	protected protectedValue: string = DEFAULT_STRING_VALUE;
	public publicValue: string = DEFAULT_STRING_VALUE;
	public readonly readonlyValue: string = DEFAULT_STRING_VALUE;
	public readonly constructorSetValue: string;

	static isMyInstance(obj: {}): boolean {
		return obj instanceof TestUserClass;
	}

	constructor(constructorArg: string) {
		this.constructorSetValue = constructorArg;
	}
}

export const TestNativeClass = EventEmitter;

export const extendUserClass = (): boolean => {
	const TSTestUserClass = getClass('TSTestUserClass');
	class ExtendedEtsUserClass extends TSTestUserClass {}
	const classInstance = new ExtendedEtsUserClass();
	return classInstance instanceof ExtendedEtsUserClass;
};

export const extendNativeClass = (): boolean => {
	const TSTestNativeClass = getClass('ArrayBuffer', 'escompat');
	class ExtendedEtsNativeClass extends TSTestNativeClass {
		constructor(length: 2) {
			super(length);
		}
	}
	const classInstance = new ExtendedEtsNativeClass(2);
	return classInstance instanceof TSTestNativeClass;
};

export const jsRespectsProtectedModifier = (): boolean => {
	const TSTestUserClass = getClass('TSTestUserClass');
	try {
		class ExtendedEtsUserClass extends TSTestUserClass {
			constructor() {
				super();
				//@ts-ignore -- to ignore compile-time check, as TSTestUserClass is only acquired in runtime
				this.protectedProperty = 'redefinedProtected';
			}

			validProtectedGetter(): string {
				//@ts-ignore -- to ignore compile-time check, as TSTestUserClass is only acquired in runtime
				return this.protectedProperty;
			}
		}
		const classInstance = new ExtendedEtsUserClass();
		return classInstance.validProtectedGetter() === 'redefinedProtected';
	} catch (e) {
		return false;
	}
};

export const jsRespectsStaticModifier = (): boolean => {
	const TSTestUserClass = getClass('TSTestUserClass');
	class ExtendedEtsUserClass extends TSTestUserClass {}
	//@ts-ignore -- to ignore compile-time check, as TSTestUserClass is only acquired in runtime
	return ExtendedEtsUserClass?.staticProperty === 'static';
};
