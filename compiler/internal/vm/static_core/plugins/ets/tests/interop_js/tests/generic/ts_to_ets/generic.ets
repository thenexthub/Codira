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

let module = ESValue.load('dynamic_import')
let tsString = module.getProperty('tsString');
let etsLiteralClass = module.getProperty('LiteralClass');
let etsUnionClass = module.getProperty('UnionClass');
let etsInterfaceClass = module.getProperty('InterfaceClass');
let etsAbstractClass = module.getProperty('AbstractClass');
let etsGClass = module.getProperty('GClass');
let etsliteralClass = module.getProperty('literalClass');
let etsunionClass = module.getProperty('unionClass');
let etsinterfaceClass = module.getProperty('interfaceClass');
let etsabstractClass = module.getProperty('abstractClass');
let etsgenericFunction = module.getProperty('genericFunction');
let etstupleDeclaredType = module.getProperty('tupleDeclaredType');
let etsexplicitlyDeclaredType = module.getProperty('explicitlyDeclaredType');

const strContent = 'ets_test_string';
const numContent = 1;
const boolContent = true;

function checkLiteralClassGeneric(): boolean {
	const literalClass = etsLiteralClass.instantiate(strContent);

	return strContent == literalClass.invokeMethod('get').toString();
}

function checkUnionClassGeneric(): boolean {
	const unionClass = etsUnionClass.instantiate(strContent);

	return strContent == unionClass.invokeMethod('get').toString();
}

function checkInterfaceClassGeneric(): boolean {
	const interfaceClass = etsInterfaceClass.instantiate(strContent);

	return strContent == interfaceClass.invokeMethod('get').toString();
}

function checkAbstractClassGeneric(): boolean {
	const abstractClass = etsAbstractClass.instantiate(strContent);

	return strContent == abstractClass.invokeMethod('get').toString();
}

function checkGenericClassGeneric(): boolean {
	const gClass = etsGClass.instantiate(strContent);

	return strContent == gClass.invokeMethod('get').toString();
}

function checkGenericLiteralClassObjectFromTs(): boolean {
	return tsString.toString() === etsliteralClass.invokeMethod('get').toString();
}

function checkGenericUnionClassObjectFromTs(): boolean {
	return tsString.toString() === etsunionClass.invokeMethod('get').toString();
}

function checkGenericInterfaceClassObjectFromTs(): boolean {
	return tsString.toString() === etsinterfaceClass.invokeMethod('get').toString();
}

function checkGenericAbstractClassObjectFromTs(): boolean {
	return tsString.toString() === etsabstractClass.invokeMethod('get').toString();
}

function checkGenericFunctionTypeAnyString(): boolean {
	return strContent == etsgenericFunction.invoke(strContent).toString();
}

function checkGenericFunctionTypeAnyInt(): boolean {
	return numContent == etsgenericFunction.invoke(numContent).toNumber().toInt();
}

function checkGenericFunctionTypeAnyBool(): boolean {
	return (boolContent as boolean) == (etsgenericFunction.invoke(boolContent).toBoolean());
}

function checkGenericFunctionTupleType() {
	const arr: [int, string] = [numContent, strContent];
	//const result = etstupleDeclaredType<[int, string]>(arr);
	const result = etstupleDeclaredType.invoke(arr);
	return  arr[0] == result.getProperty(0).toNumber() && arr[1] == result.getProperty(1).toString();
}

function checkGenericExplicitlyDeclaredType(): boolean {
	return ESValue.areEqualSafe(tsString, etsexplicitlyDeclaredType.invoke());
}
