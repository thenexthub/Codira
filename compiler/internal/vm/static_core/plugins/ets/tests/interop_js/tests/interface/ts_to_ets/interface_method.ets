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

let module = ESValue.load('dynamic_import');
let tsNumber = module.getProperty('tsNumber').toNumber();
let tsString = module.getProperty('tsString').toString();
let AnyTypeMethodClass = module.getProperty('AnyTypeMethodClass');
let createInterfaceClassAnyTypeMethod = module.getProperty('createInterfaceClassAnyTypeMethod');
let UnionTypeMethodClass = module.getProperty('UnionTypeMethodClass');
let createInterfaceClassUnionTypeMethod = module.getProperty('createInterfaceClassUnionTypeMethod');
let unionTypeMethodInstanceClass = module.getProperty('unionTypeMethodInstanceClass');
let subsetByRefInterface = module.getProperty('subsetByRefInterface');
let SubsetByValueClass = module.getProperty('SubsetByValueClass');
let createSubsetByValueClassFromTs = module.getProperty('createSubsetByValueClassFromTs');
let subsetByValueInstanceClass = module.getProperty('subsetByValueInstanceClass');
let TupleTypeMethodClass = module.getProperty('TupleTypeMethodClass');
let createInterfaceClassTupleTypeMethodFromTs = module.getProperty('createInterfaceClassTupleTypeMethodFromTs');
let tupleInstanceClass = module.getProperty('tupleInstanceClass');
let WithOptionalMethodClass = module.getProperty('WithOptionalMethodClass');
let WithoutOptionalMethodClass = module.getProperty('WithoutOptionalMethodClass');
let createClassWithOptionalMethod = module.getProperty('createClassWithOptionalMethod');
let createClassWithoutOptionalMethod = module.getProperty('createClassWithoutOptionalMethod');
let withoutOptionalMethodInstanceClass = module.getProperty('withoutOptionalMethodInstanceClass');
let withOptionalMethodInstanceClass = module.getProperty('withOptionalMethodInstanceClass');
let optionalArg = module.getProperty('optionalArg');
let optionalArgArray = module.getProperty('optionalArgArray');

const str: string = 'string';
const bool: boolean = true;
const num: int = 1;
const arr: int[] = [];
const obj: Object = {};
const arrTuple: [int, string] = [num, str];

class ObjMethod {
	get(): int {
		return num
	}
}

function checkAnyTypeInterfaceClassString(): boolean {
	const anyTypeMethodClass = AnyTypeMethodClass.instantiate();

	return (str as string) === (anyTypeMethodClass.invokeMethod('get', str).toString());
}

function checkAnyTypeInterfaceClassInt(): boolean {
	const anyTypeMethodClass = AnyTypeMethodClass.instantiate();

	return (num as int) === (anyTypeMethodClass.invokeMethod('get', num).toNumber());
}

function checkAnyTypeInterfaceClassBool(): boolean {
	const anyTypeMethodClass = AnyTypeMethodClass.instantiate();

	return (bool as boolean) === (anyTypeMethodClass.invokeMethod('get', bool).toBoolean());
}

function checkAnyTypeInterfaceClassArray(): boolean {
	const anyTypeMethodClass = AnyTypeMethodClass.instantiate();

	return (anyTypeMethodClass.invokeMethod('get', arr).unwrap() as Array<int>) instanceof Array;
}

function checkAnyTypeInterfaceClassObject(): boolean {
	const anyTypeMethodClass = AnyTypeMethodClass.instantiate()

	return 'object' === (anyTypeMethodClass.invokeMethod('get', obj).typeOf());
}

function checkCreateInterfaceClassAnyTypeMethodString(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod.invoke();

	return (str as string) === (anyTypeMethodClass.invokeMethod('get', str).toString());
}

function checkCreateInterfaceClassAnyTypeMethodInt(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod.invoke();

	return (num as int) === (anyTypeMethodClass.invokeMethod('get', num).toNumber());
}

function checkCreateInterfaceClassAnyTypeMethodBool(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod.invoke();

	return (bool as boolean) === (anyTypeMethodClass.invokeMethod('get', bool).toBoolean());
}

function checkCreateInterfaceClassAnyTypeMethodArray(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod.invoke();

	return (anyTypeMethodClass.invokeMethod('get', arr).unwrap() as Array<int>) instanceof Array;
}

function checkCreateInterfaceClassAnyTypeMethodObject(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod.invoke();

	return 'object' === (anyTypeMethodClass.invokeMethod('get', obj).typeOf() as Object);
}

function checkUnionTypeInterfaceClassString(): boolean {
	const unionTypeMethodClass = UnionTypeMethodClass.instantiate();

	return (str as string) === (unionTypeMethodClass.invokeMethod('get', str).toString());
}

function checkUnionTypeInterfaceClassInt(): boolean {
	const unionTypeMethodClass = UnionTypeMethodClass.instantiate();

	return (num as int) === (unionTypeMethodClass.invokeMethod('get', num).toNumber());
}

function checkCreateInterfaceClassUnionTypeMethodString(): boolean {
	const unionTypeMethodClass = createInterfaceClassUnionTypeMethod.invoke();

	return (str as string) === (unionTypeMethodClass.invokeMethod('get', str).toString());
}

function checkCreateInterfaceClassUnionTypeMethodInt(): boolean {
	const unionTypeMethodClass = createInterfaceClassUnionTypeMethod.invoke();

	return (num as int) === (unionTypeMethodClass.invokeMethod('get', num).toNumber());
}

function checkInstanceInterfaceClassUnionTypeMethodString(): boolean {
	return (str as string) === (unionTypeMethodInstanceClass.invokeMethod('get', str).toString());
}

function checkInstanceInterfaceClassUnionTypeMethodInt(): boolean {
	return (num as int) === (unionTypeMethodInstanceClass.invokeMethod('get', num).toNumber());
}

function checkSubsetByRefInterface(): boolean {
	return (num as int) === (subsetByRefInterface.invoke(new ObjMethod()).toNumber());
}

function checkSubsetByRefInterfaceError(): boolean {
	try {
		subsetByRefInterface.invoke(obj);
		return false;
	} catch (e) {
		return true;
	}
}

function checkSubsetByValueClass(): boolean {
	const subsetByValueClass = SubsetByValueClass.instantiate();
	const userClass = subsetByValueClass.invokeMethod('get');

	return (tsNumber.toInt()) === (userClass.getProperty('value').toNumber());
}

function checkCreateSubsetByValueClassFromTs(): boolean {
	const subsetByValueClass = createSubsetByValueClassFromTs.invoke();
	const userClass = subsetByValueClass.invokeMethod('get');

	return (tsNumber.toInt()) === (userClass.getProperty('value').toNumber());
}

function checkInstanceSubsetByValueClassFromTs(): boolean {
	const userClass = subsetByValueInstanceClass.invokeMethod('get');

	return (tsNumber.toInt()) === (userClass.getProperty('value').toNumber());
}

function checkTupleTypeClass(): boolean {
	const tupleClass = TupleTypeMethodClass.instantiate()

	const result = tupleClass.invokeMethod('get', arrTuple);
	return result.unwrap() as Array<[int, string]> instanceof Array &&
		   result.getProperty(0).toNumber() === (num as int) &&
		   result.getProperty(0).toString() === str as string;
}

function checkCreateTupleClassFromTs(): boolean {
	const tupleClass = createInterfaceClassTupleTypeMethodFromTs.invoke();

	const result = tupleClass.invokeMethod('get', arrTuple);
	return result.unwrap() as Array<[int, string]> instanceof Array &&
		   (result.getProperty(0).toNumber()) === (num as int) &&
		   (result.getProperty(1).toString()) === (str as string);
}

function checkInstanceTupleClassFromTs(): boolean {
	const result = tupleInstanceClass.invokeMethod('get', arrTuple);
	return result.unwrap() as Array<[int, string]> instanceof Array &&
		   (result.getProperty(0).toNumber()) === (num as int) &&
		   (result.getProperty(1).toString()) === (str as string);
}

function checkWithOptionalMethodClass(): boolean {
	const optionalClass = WithOptionalMethodClass.instantiate();

	return (optionalClass.invokeMethod('getNum').toNumber()) === (tsNumber.toInt()) &&
		   (optionalClass.invokeMethod('getStr').toString()) === (tsString as string);
}

function checkWithoutOptionalMethodClass(): boolean {
	const optionalClass = WithoutOptionalMethodClass.instantiate();

	return (optionalClass.invokeMethod('getStr').toString()) === (tsString as string);
}

function checkCreateClassWithOptionalMethod() {
	const optionalClass = createClassWithOptionalMethod.invoke();

	return (optionalClass.invokeMethod('getNum').toNumber()) === (tsNumber.toInt()) &&
		   (optionalClass.invokeMethod('getStr').toString()) === (tsString as string);
}

function checkCreateClassWithoutOptionalMethod() {
	const optionalClass = createClassWithoutOptionalMethod.invoke();

	return (optionalClass.invokeMethod('getStr').toString()) === (tsString as string);
}

function checkWithOptionalMethodInstanceClass(): boolean {
	let resNum = (withOptionalMethodInstanceClass.invokeMethod('getNum').toNumber());
	let resStr = (withOptionalMethodInstanceClass.invokeMethod('getStr').toString());
	return resNum === (tsNumber.toInt()) && resStr === (tsString as string);
}

function checkWithoutOptionalMethodInstanceClass() {
	let resStr = (withoutOptionalMethodInstanceClass.invokeMethod('getStr').toString());
	return resStr === (tsString as string);
}

function checkOptionalArgWithAllArgs(): boolean {
	const result = optionalArg.invoke(withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass);

	return !!result.getProperty('with').unwrap() && !!result.getProperty('without').unwrap();
}

function checkOptionalArgWithOneArgs(): boolean {
	const result = optionalArg.invoke(withOptionalMethodInstanceClass);

	return !!result.getProperty('with').unwrap();
}

// NOTE (issues 17772) fix spread operator and
// uncomment function in checkSpreadOperatorArgWithAllArgs

// function checkSpreadOperatorArgWithAllArgs(): boolean {
//     const arr: FixedArray<WithoutOptionalMethodClass | withoutOptionalMethodInstanceClass> = [withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass];
//     const result = optionalArgArray(...arr);
//     return !!result.with && !!result.without;
// }

// function checkSpreadOperatorArgWithOneArgs(): boolean {
//     const arr: FixedArray<WithoutOptionalMethodClass | WithoutOptionalMethodClass> = [withOptionalMethodInstanceClass];
//     const result = optionalArgArray(...arr);
//     return !!result.with;
// }

function checkWithoutSpreadOperatorArgWithAllArgs(): boolean {
	const result = optionalArgArray.invoke(withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass)
	return !!result.getProperty('with').unwrap() && !!result.getProperty('without').unwrap();
}

function checkWithoutSpreadOperatorArgWithOneArgs(): boolean {
	const result = optionalArgArray.invoke(withOptionalMethodInstanceClass)
	return !!result.getProperty('with').unwrap();
}
