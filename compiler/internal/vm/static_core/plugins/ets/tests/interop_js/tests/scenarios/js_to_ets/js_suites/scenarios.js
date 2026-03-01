/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
'use strict';

const STRING_VALUE = '1';
const INT_VALUE = 1;
const INT_VALUE2 = 2;
const INT_VALUE3 = 3;
const FLOAT_VALUE = 1.0;


export function standaloneFunctionJs() {
	return 1;
}

export class ClassWithMethodJs {
	methodInClassJs() {
		return 1;
	}
}

export class InterfaceWithMethodImpl {
	methodInInterface() {
		return 1;
	}
}

export function newInterfaceWithMethod() {
	let implInterfaceWithMethod = new InterfaceWithMethodImpl();
	return implInterfaceWithMethod;
	/* above is transpiled from the following TS code:
    interface InterfaceWithMethod {
      methodInInterface(): int;
    }

    export class InterfaceWithMethodImpl implements InterfaceWithMethod {
      methodInInterface(): int {
        return 1;
      }
    }

    export function newInterfaceWithMethod(): InterfaceWithMethod {
      let implInterfaceWithMethod: InterfaceWithMethod = new InterfaceWithMethodImpl();
      return implInterfaceWithMethod;
    }
  */
}

export class ClassWithGetterSetter {
	_value = 1;

	get value() {
		return this._value;
	}

	set value(theValue) {
		this._value = theValue;
	}
}

export let lambdaFunction = () => {
	return 1;
};

export function genericFunction(arg) {
	return arg;
}

export function genericTypeParameter(arg) {
	return arg.toString();
}

export function genericTypeReturnValue(arg) {
	return arg;
}

export class ClassToExtend {
	value() {
		return 1;
	}
}

// NOTE(oignatenko) return and arg types any, unknown, undefined need real TS because transpiling cuts off
//   important details. Have a look at declgen_ets2ts
export function functionArgTypeAny(arg) {
	return arg; // transpiled from Typescript code: functionArgTypeAny(arg: any)
}

export function functionArgTypeUnknown(arg) {
	return arg; // transpiled from Typescript code: functionArgTypeUnknown(arg: unknown)
}

export function functionArgTypeUndefined(arg) {
	return arg; // transpiled from Typescript code: functionArgTypeUndefined(arg: undefined)
}

export function functionArgTypeTuple(arg) {
	return arg[0]; // transpiled from Typescript code: functionArgTypeTuple(arg: [number, string]): number
}

export function functionReturnTypeAny() {
	let value = 1;
	return value; // transpiled from Typescript code:functionReturnTypeAny(): any
}

export function functionReturnTypeUnknown() {
	let value = 1;
	return value; // transpiled from Typescript code: functionReturnTypeUnknown(): unknown
}

export function functionReturnTypeUndefined() {
	let value = 1;
	return value; // transpiled from Typescript code: functionReturnTypeUndefined(): undefined
}

export function functionArgTypeCallable(functionToCall) {
	return functionToCall();
	// transpiled from Typescript code: unctionArgTypeCallable(functionToCall: () => number): number
}

export function functionDefaultParameterFunction(arg1 = INT_VALUE, arg2 = INT_VALUE2, arg3 = INT_VALUE3) {
	let value = 1;
	return value;
	// transpiled from Typescript code:
	// functionDefaultParameterFunction(
	//	arg1: JSValue = INT_VALUE,
	//	arg2: JSValue = INT_VALUE2,
	//	arg3: JSValue = INT_VALUE3): int
}

export function functionDefaultIntParameterFunction(arg = INT_VALUE) {
	return arg;
	// transpiled from Typescript code: export function defaultFloatParameterFunction(arg: JSValue = INT_VALUE): JSValue
}

export function functionDefaultStringParameterFunction(arg = STRING_VALUE) {
	return arg;
	// transpiled from Typescript code: functionDefaultStringParameterFunction(arg: JSValue = STRING_VALUE): JSValue{
}

export function functionDefaultFloatParameterFunction(arg = FLOAT_VALUE) {
	return arg;
	// transpiled from Typescript code: export function defaultFloatParameterFunction(arg: JSValue = FLOAT_VALUE): JSValue{
}

export function functionArgTypeOptionalPrimitive(arg) {
	if (typeof arg !== 'undefined') {
		return arg;
	}
	return INT_VALUE;
}

export function functionArgTypePrimitive(arg) {
	return arg;
}

export function functionReturnTypePrimitive() {
	return true;
}

export function optionalCall(x = 123, y = 130, z = 1) {
	return x + y + z;
}

export function singleRequired(z, x = 123, y = 123) {
	return x + y + z;
}

export function jsSumRestParams(...args) {
	let sum = 0;
	args.forEach((n) => (sum += n));
	return sum;
}

export function jsMultiplyArgBySumRestParams(arg0, ...args) {
	let sum = 0;
	args.forEach((n) => (sum += n));
	return sum * arg0;
}

export function jsMultiplySumArgsBySumRestParams(arg0, arg1, ...args) {
	let sum = 0;
	args.forEach((n) => (sum += n));
	return sum * (arg0 + arg1);
}

export function jsConcatStringsRestParams(...args) {
	let str = '';
	args.forEach((s) => (str += s));
	return str;
}

export class InterfaceWithUnionImpl {
	methodInInterface(arg) {
		return functionReturnTypeUnion(arg);
	}
}

export function newInterfaceWithUnion() {
	return new InterfaceWithUnionImpl();
}

export function functionArgTypeUnion(arg) {
	switch (typeof arg) {
		case 'number':
			return 0;
		case 'string':
			return 1;
		default:
			return -1;
	}
}

export function functionReturnTypeUnion(arg) {
	if (arg === 0) {
		return INT_VALUE;
	}
	return STRING_VALUE;
}

export class UnionTestClassJs {
	methodArgTypeUnion(arg) {
		return functionArgTypeUnion(arg);
	}
	methodReturnTypeUnion(arg) {
		return functionReturnTypeUnion(arg);
	}
}

export class ClassWithStaticMethod {
	static staticMethod(arg) {
		return arg;
	}
	static staticMethodReturnTypeUnion(arg) {
		return functionReturnTypeUnion(arg);
	}
}

export class ClassReturnThis {
  returnThis() {
    return this;
  }
}

export function functionReturnOmittedValue() {}

export function functionRestParameter(...arg) {
	return arg[0]; // transpiled from Typescript code: export function functionRestParameter(...arg: number[])
}

export function functionSpreadParameter(arg1, arg2) {
	return arg1 + arg2; // transpiled from Typescript code: export function functionSpreadParameter(arg1: number, arg2: number)
}
export function functionArgStringLiteralType(arg) {
	return arg;
	// transpiled from Typescript code: functionArgStringLiteralType(arg: TypeString): TypeString
}

export function functionIntersectionTypePrimitive(arg) {
	const ret = arg;
	return ret;
	// transpiled from Typescript code: functionIntersectionTypePrimitive(arg: PrimitiveAB): PrimitiveAB
}

export class ClassWithInFieldDeclaration {
    constructor(value) {
        this.value = value;
    }
}

export function functionReturnsCompositeType(arg) {
    switch (arg) {
      case 0:
        return STRING_VALUE;
      case 1:
        return FLOAT_VALUE;
      case 2:
        return INT_VALUE;
      default:
        return ['arrayvalue1', 'arrayvalue2'];
    }
}
