/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
'use strict';

export function evaluateNumber(v0, v1) {
	return v0 + v1;
}

export function doNothing() {
	print('Hello from empty function');
}

export function evaluateObject(obj) {
	return new ExampleClass(obj.v0 + obj.v1, obj.v0 + obj.v1);
}

export function evaluateArray(arr, size) {
	let result = [];
	for (let i = 0; i < size; i++) {
		result[i] = arr[i] + i * i;
	}
	return result;
}

export class ExampleClass {
	constructor(v0, v1) {
		this.v0 = v0;
		this.v1 = v1;
	}

	static evaluateNumber(v2, v3) {
		return v2 + v3;
	}

	static evaluateArray(arr, size) {
		let result = [];
		for (let i = 0; i < size; i++) {
			result[i] = arr[i] + i * i;
		}
		return result;
	}

	instanceEvaluateNumber() {
		// without 'this' also not working
		return this.v0 + this.v1;
	}

	evaluateObject(obj) {
		return new ExampleClass(obj.v0 + this.v1, this.v0 + obj.v1);
	}

	getV0() {
		return this.v0;
	}

	getV1() {
		return this.v1;
	}

	static emptyMethod() {
		print('Hello from empty method');
	}
}

export class ClassWithEmptyConstructor {
	constructor() {
		this.v0 = 42;
		this.v1 = 42;
	}

	verifyProperties(v0Expected, v1Expected) {
		if (this.v0 === v0Expected && this.v1 === v1Expected) {
			return 0;
		} else {
			return 1;
		}
	}

	getV0() {
		return this.v0;
	}

	getV1() {
		return this.v1;
	}
}

export class Kitten {
    constructor(id, name) {
      this.id = id;
      this.name = name;
      this.male = false;
    }
  }

export const createKitten = (id, name) => new Kitten(id, name);

export const MyNamespace = {
    Kitten,
    createKitten
};
