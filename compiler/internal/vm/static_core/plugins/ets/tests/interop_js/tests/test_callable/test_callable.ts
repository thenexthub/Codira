/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

export function callNamedFunction(x: number, y: number): number {
	return x + y;
}

export let callAnonymousFunction = function (x: number, y: number): number {
	return x + y;
};

export let callArrowFunction = (x: number, y: number): Object => x + y;

// Enable when #24130 is fixed
// export const callConstructedFunction = new Function('a', 'b', 'return a + b');

export let callBoundFunction = callNamedFunction.bind(null);

export class CallableTestClass {
	public callNamedFunction = callNamedFunction.bind(this);
	public callAnonymousFunction = callAnonymousFunction.bind(this);
	public callArrowFunction = callArrowFunction.bind(this);
	// Enable when #24130 is fixed
	// public callConstructedFunction = callConstructedFunction.bind(this);
	public callBoundFunction = callBoundFunction.bind(this);
}

export type TCallableTestClass = CallableTestClass;
export const callableTestClassInstance = new CallableTestClass();

// Enable when #24130 is fixed
// const test = (): void => {
// 	print(`${[callNamedFunction, callAnonymousFunction, callArrowFunction, callConstructedFunction, callBoundFunction].map((x) => x(10, 5))}`);
// };

// Remove when #24130 is fixed
const test = (): void => {
	print(`${[callNamedFunction, callAnonymousFunction, callArrowFunction, callBoundFunction].map((x) => x(10, 5))}`);
};

test();
