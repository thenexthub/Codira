/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export class TestUserClass {
	public name: string;

	private age: number;

	protected id: number;

	public readonly education: string;

	private description: string;

	constructor(name: string, age: number, id: number, education: string, description: string) {
		this.name = name;
		this.age = age;
		this.id = id;
		this.description = description;
		this.education = education;
	}

	public getDetails(): string {
		return `Name: ${this.name}, Age: ${this.age}, ID: ${this.id}, Description: ${this.description}`;
	}

	protected getProtectedInfo(): string {
		return `ID: ${this.id}`;
	}
}

export class TestSecondClass extends TestUserClass {
	public getProtectedId(): number {
		return this.id;
	}

	public getInfo(): string {
		return this.getProtectedInfo();
	}
}

export interface TestObjectType {
	name: string;
	id: number;
}

export const testObject: TestObjectType = {
	name: 'TestName',
	id: 555,
};

export function getName(obj: TestUserClass): string {
	return obj.name;
}

export function getDetails(obj: TestUserClass): string {
	return obj.getDetails();
}

export function changeName(obj: TestUserClass, name: string): Object {
	obj.name = name;
	return obj.name;
}

export function getEdu(obj: TestUserClass): string {
	return obj.education;
}

export function getId(obj: TestSecondClass): number {
	return obj.getProtectedId();
}

export function getObjectName(obj: TestObjectType): Object {
	return obj.name;
}

export function getObjectId(obj: TestSecondClass): Object {
	return obj.getProtectedId();
}

export const testObjCls = new TestUserClass('TestName', 30, 456, 'testEdu', 'testDescription');
export const testSecondObjCls = new TestSecondClass('TestNameTwo', 40, 789, 'testEdu1', 'testDescription1');

const testInnerObject = { id: 123 };

export const testOuterObject = { id: 456, testInnerObject };

export function getOuterObj(obj): Object {
	return obj.testInnerObject.id;
}

export function updateObjectId(obj: TestObjectType, newId: number): void {
	obj.id = newId;
}
