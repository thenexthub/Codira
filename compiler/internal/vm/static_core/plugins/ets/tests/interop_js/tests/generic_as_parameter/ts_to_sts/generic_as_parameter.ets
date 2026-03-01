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

let module = ESObject.load('../../plugins/ets/tests/interop_js/tests/generic_as_parameter/ts_to_sts/generic_as_parameter');

let tsNumber = module.getProperty('tsNumber');
let tsString = module.getProperty('tsString');
let tsBool = module.getProperty('tsBool');
let tsArr = module.getProperty('tsArr');
let tsObj = module.getProperty('tsObj');
let tsUnion = module.getProperty('tsUnion');
let tsTuple = module.getProperty('tsTuple');
let tsLiteral = module.getProperty('tsLiteral');
let anyTypeParameter = module.getProperty('anyTypeParameter');
let anyTypeParameterExplicitCallFromTsInt = module.getProperty('anyTypeParameterExplicitCallFromTsInt');
let anyTypeParameterExplicitCallFromTsString = module.getProperty('anyTypeParameterExplicitCallFromTsString');
let anyTypeParameterExplicitCallFromTsBool = module.getProperty('anyTypeParameterExplicitCallFromTsBool');
let anyTypeParameterExplicitCallFromTsArr = module.getProperty('anyTypeParameterExplicitCallFromTsArr');
let anyTypeParameterExplicitCallFromTsObj = module.getProperty('anyTypeParameterExplicitCallFromTsObj');
let anyTypeParameterExplicitCallFromTsUnion = module.getProperty('anyTypeParameterExplicitCallFromTsUnion');
let anyTypeParameterExplicitCallFromTsTuple = module.getProperty('anyTypeParameterExplicitCallFromTsTuple');
let anyTypeParameterExplicitCallFromTsLiteral = module.getProperty('anyTypeParameterExplicitCallFromTsLiteral');
let genericTypeFunctionReturnAny = module.getProperty('genericTypeFunctionReturnAny');
let genericTypeFunctionExplicitCallFromTsInt = module.getProperty('genericTypeFunctionExplicitCallFromTsInt');
let genericTypeFunctionExplicitCallFromTsString = module.getProperty('genericTypeFunctionExplicitCallFromTsString');
let genericTypeFunctionExplicitCallFromTsBool = module.getProperty('genericTypeFunctionExplicitCallFromTsBool');
let genericTypeFunctionExplicitCallFromTsArr = module.getProperty('genericTypeFunctionExplicitCallFromTsArr');
let genericTypeFunctionExplicitCallFromTsObj = module.getProperty('genericTypeFunctionExplicitCallFromTsObj');
let genericTypeFunctionExplicitCallFromTsUnion = module.getProperty('genericTypeFunctionExplicitCallFromTsUnion');
let genericTypeFunctionExplicitCallFromTsTuple = module.getProperty('genericTypeFunctionExplicitCallFromTsTuple');
let genericTypeFunctionExplicitCallFromTsLiteral = module.getProperty('genericTypeFunctionExplicitCallFromTsLiteral');
let extendGenericNumber = module.getProperty('extendGenericNumber');
let extendGenericString = module.getProperty('extendGenericString');
let extendGenericBool = module.getProperty('extendGenericBool');
let extendGenericArr = module.getProperty('extendGenericArr');
let extendGenericObj = module.getProperty('extendGenericObj');
let extendGenericUnion = module.getProperty('extendGenericUnion');
let extendGenericTuple = module.getProperty('extendGenericTuple');
let extendGenericLiteral = module.getProperty('extendGenericLiteral');
let GInterfaceClass = module.getProperty('GInterfaceClass');
let GTypeClass = module.getProperty('GTypeClass');
let genericExtendInterface = module.getProperty('genericExtendInterface');
let genericExtendType = module.getProperty('genericExtendType');
let tupleGeneric = module.getProperty('tupleGeneric');
let collectGeneric = module.getProperty('collectGeneric');
let UserClass = module.getProperty('UserClass');
let createClassFromTs = module.getProperty('createClassFromTs');
let userClassInstance = module.getProperty('userClassInstance');
let genericDefaultInt = module.getProperty('genericDefaultInt');
let genericDefaultString = module.getProperty('genericDefaultString');
let genericDefaultBool = module.getProperty('genericDefaultBool');
let genericDefaultArr = module.getProperty('genericDefaultArr');
let genericDefaultObj = module.getProperty('genericDefaultObj');
let genericDefaultUnion = module.getProperty('genericDefaultUnion');
let genericDefaultTuple = module.getProperty('genericDefaultTuple');
let genericDefaultLiteral = module.getProperty('genericDefaultLiteral');
let genericDefaultIntCallFromTs = module.getProperty('genericDefaultIntCallFromTs');
let genericDefaultStringCallFromTs = module.getProperty('genericDefaultStringCallFromTs');
let genericDefaultBoolCallFromTs = module.getProperty('genericDefaultBoolCallFromTs');
let genericDefaultArrCallFromTs = module.getProperty('genericDefaultArrCallFromTs');
let genericDefaultObjCallFromTs = module.getProperty('genericDefaultObjCallFromTs');
let genericDefaultUnionCallFromTs = module.getProperty('genericDefaultUnionCallFromTs');
let genericDefaultTupleCallFromTs = module.getProperty('genericDefaultTupleCallFromTs');
let genericDefaultLiteralCallFromTs = module.getProperty('genericDefaultLiteralCallFromTs');
let DataClass = module.getProperty('DataClass');
let genericExtendClass = module.getProperty('genericExtendClass');

type TupleT = [int, boolean];
const etsNumber: number = 1;
const etsString: string = 'string';
const etsBool: boolean = true;
const etsArr = [etsNumber];
const etsObj: object = {};
const etsUnion = etsNumber;
const etsTuple = [etsNumber, etsBool];

function checkAnyTypeParameterInt(): boolean {
    return anyTypeParameter.invoke(etsNumber).toNumber() == etsNumber;
}

function checkAnyTypeParameterString(): boolean {
    return anyTypeParameter.invoke(etsString).toString() == etsString.toString();
}

function checkAnyTypeParameterBool(): boolean {
    return anyTypeParameter.invoke(etsBool).toBoolean() == etsBool.toBoolean();
}

function checkAnyTypeParameterArr(): boolean {
    const res = anyTypeParameter.invoke(etsArr);
    return res.getProperty(0).toNumber() == etsArr[0];
}

function checkAnyTypeParameterObj(): boolean {
    return anyTypeParameter.invoke(etsObj).typeOf() == 'object';
}

function checkAnyTypeParameterUnion(): boolean {
    return anyTypeParameter.invoke(etsUnion).toNumber() == etsNumber;
}

function checkAnyTypeParameterTuple(): boolean {
    const res = anyTypeParameter.invoke(etsTuple);

    return res.getProperty(0).toNumber() == etsTuple[0]
        && res.getProperty(1).toBoolean() == etsTuple[1];
}
// any type parameter explicit

function checkAnyTypeParameterExplicitCallInt(): boolean {
    return anyTypeParameterExplicitCallFromTsInt.invoke().toNumber() == tsNumber.toNumber();
}

function checkAnyTypeParameterExplicitCallString(): boolean {
    return anyTypeParameterExplicitCallFromTsString.invoke().toString() == tsString.toString();
}

function checkAnyTypeParameterExplicitCallBool(): boolean {
    return anyTypeParameterExplicitCallFromTsBool.invoke().toBoolean() == tsBool.toBoolean();
}

function checkAnyTypeParameterExplicitCallArr(): boolean {
    const res = anyTypeParameterExplicitCallFromTsArr.invoke();
    return res.getProperty(0).toNumber() == tsArr.getProperty(0).toNumber();
}

function checkAnyTypeParameterExplicitCallObj(): boolean {
    return anyTypeParameterExplicitCallFromTsObj.invoke().typeOf() == 'object';
}

function checkAnyTypeParameterExplicitCallUnion(): boolean {
    return anyTypeParameterExplicitCallFromTsUnion.invoke().toNumber() == tsUnion.toNumber();
}

function checkAnyTypeParameterExplicitCallTuple(): boolean {
    const res = anyTypeParameterExplicitCallFromTsTuple.invoke();
    return res.getProperty(0).toNumber() == tsTuple.getProperty(0).toNumber()
        && res.getProperty(1).toBoolean() == tsTuple.getProperty(1).toBoolean();
}

function checkAnyTypeParameterExplicitCallLiteral(): boolean {
    return anyTypeParameterExplicitCallFromTsLiteral.invoke().toNumber() == tsLiteral.toNumber();
}

// generic type function return any
function checkGenericTypeFunctionReturnAnyInt(): boolean {
    return genericTypeFunctionReturnAny.invoke(etsNumber).toNumber() == etsNumber;
}

function checkGenericTypeFunctionReturnAnyString(): boolean {
    return genericTypeFunctionReturnAny.invoke(etsString).toString() == etsString;
}

function checkGenericTypeFunctionReturnAnyBool(): boolean {
    return genericTypeFunctionReturnAny.invoke(etsBool).toBoolean() == etsBool.toBoolean();
}

function checkGenericTypeFunctionReturnAnyArr(): boolean {
    const res = genericTypeFunctionReturnAny.invoke(etsArr);
    return res.getProperty(0).toNumber() == etsArr[0];
}

function checkGenericTypeFunctionReturnAnyObj(): boolean {
    return genericTypeFunctionReturnAny.invoke(etsObj).typeOf() == 'object';
}

function checkGenericTypeFunctionReturnAnyUnion(): boolean {
    return genericTypeFunctionReturnAny.invoke(etsUnion).toNumber() == etsNumber;
}

function checkGenericTypeFunctionReturnAnyTuple(): boolean {
    const res = genericTypeFunctionReturnAny.invoke(etsTuple);

    return res.getProperty(0).toNumber() == etsTuple[0]
        && res.getProperty(1).toNumber() == etsTuple[1];
}
//
function checkGenericTypeFunctionExplicitCallInt(): boolean {
    return genericTypeFunctionExplicitCallFromTsInt.invoke().toNumber() == tsNumber.toNumber();
}

function checkGenericTypeFunctionExplicitCallString(): boolean {
    return genericTypeFunctionExplicitCallFromTsString.invoke().toString() == tsString.toString();
}

function checkGenericTypeFunctionExplicitCallBool(): boolean {
    return genericTypeFunctionExplicitCallFromTsBool.invoke().toBoolean() == tsBool.toBoolean();
}

function checkGenericTypeFunctionExplicitCallArr(): boolean {
    const res = genericTypeFunctionExplicitCallFromTsArr.invoke();
    return res.getProperty(0).toNumber() == tsArr.getProperty(0).toNumber();
}

function checkGenericTypeFunctionExplicitCallObj(): boolean {
    return genericTypeFunctionExplicitCallFromTsObj.invoke().typeOf() == 'object';
}

function checkGenericTypeFunctionExplicitCallUnion(): boolean {
    return genericTypeFunctionExplicitCallFromTsUnion.invoke().toNumber() == tsUnion.toNumber();
}

function checkGenericTypeFunctionExplicitCallTuple(): boolean {
    const res = genericTypeFunctionExplicitCallFromTsTuple.invoke();
    return res.getProperty(0).toNumber() == tsTuple.getProperty(0).toNumber()
        && res.getProperty(1).toBoolean() == tsTuple.getProperty(1).toBoolean();
}

function checkGenericTypeFunctionExplicitCallLiteral(): boolean {
    return genericTypeFunctionExplicitCallFromTsLiteral.invoke().toNumber() == tsLiteral.toNumber();
}
// extends generic arg
function checkExtendGenericNumber(): boolean {
    return extendGenericNumber.invoke(etsNumber).toNumber() == etsNumber;
}

function checkExtendGenericString(): boolean {
    return extendGenericString.invoke(etsString).toString() == etsString.toString();
}

function checkExtendGenericBool(): boolean {
    return extendGenericBool.invoke(etsBool).toBoolean() == etsBool.toBoolean();
}

function checkExtendGenericArr(): boolean {
    const res = extendGenericArr.invoke(etsArr);
    return res.getProperty(0).toNumber() == etsArr[0];
}

function checkExtendGenericObj(): boolean {
    return extendGenericObj.invoke(etsObj).typeOf() == 'object';
}

function checkExtendGenericUnion(): boolean {
    return extendGenericUnion.invoke(etsUnion).toNumber() == etsUnion;
}

function checkExtendGenericTuple(): boolean {
    const res = extendGenericTuple.invoke(etsTuple);

    return res.getProperty(0).toNumber() == etsTuple[0]
        && res.getProperty(1).toNumber() == etsTuple[1];
}

function checkExtendGenericLiteral(): boolean {
    return extendGenericLiteral.invoke(tsLiteral).toNumber() == tsLiteral.toNumber();
}
// generic extend type and interface
function checkGenericExtendInterface(): boolean {
    const obj = GTypeClass.instantiate();
  
    const res = genericExtendInterface.invoke(obj);
    return res.getPropertySafe("data").toString() == obj.getPropertySafe("data").toString();
}

function checkGenericExtendType(): boolean {
    const obj = GTypeClass.instantiate();
    const res = genericExtendType.invoke(obj);
    return !!res.getPropertySafe("data") && res.getPropertySafe("data").toString() == obj.getPropertySafe("data").toString();
}

function checkTupleGeneric() {
    const res = tupleGeneric.invoke(etsTuple);

    return res.getProperty(0).toNumber() == etsTuple[0]
        && res.getProperty(1).toNumber() == etsTuple[1];
}
// collectGeneric

function checkCollectGenericInt(): boolean {
    const res = collectGeneric.invoke(etsNumber);
    return res.getProperty(0).toNumber() == etsNumber;
}

function checkCollectGenericString(): boolean {
    const res = collectGeneric.invoke(etsString);

    return res.getProperty(0).toString() == etsString;
}

function checkCollectGenericBool(): boolean {
    const res = collectGeneric.invoke(etsBool);

    return res.getProperty(0).toBoolean() == etsBool.toBoolean();
}

function checkCollectGenericObj(): boolean {
    const res = collectGeneric.invoke(tsObj);
    return res.isObject();
}

//user class
function checkGenericUserClass(): boolean {
    const user = UserClass.instantiate(etsString, etsNumber);
    return user.getPropertySafe("name").toString() == etsString
           && user.getPropertySafe("age").toNumber() == etsNumber;
}

function checkCreateClassFromTs(): boolean {
    const user = createClassFromTs.invoke();

    return user.getPropertySafe('name').toString() == tsString.toString()
           && user.getPropertySafe('age').toNumber() == tsNumber.toNumber();
}

function checkUserClassInstance(): boolean {
    const user = userClassInstance;

    return user.getPropertySafe("name").toString() == tsString.toString() 
           && user.getPropertySafe("age").toNumber() == tsNumber.toNumber();
}
// generic default type
function checkGenericDefaultInt(): boolean {
    return genericDefaultInt.invoke(etsNumber).toNumber() == etsNumber;
}

function checkGenericDefaultString(): boolean {
    return genericDefaultString.invoke(etsString).toString() == etsString.toString();
}

function checkGenericDefaultBool(): boolean {
    return genericDefaultBool.invoke(etsBool).toBoolean() == etsBool.toBoolean();
}

function checkGenericDefaultArr(): boolean {
    const res = genericDefaultArr.invoke(etsArr);

    return res.getProperty(0).toNumber() == etsArr[0];
}

function checkGenericDefaultObj(): boolean {
    return genericDefaultObj.invoke(etsObj).typeOf() == 'object';
}

function checkGenericDefaultUnion(): boolean {
    return genericDefaultUnion.invoke(etsUnion).toNumber() == etsNumber;
}

function checkGenericDefaultTuple(): boolean {
    const res = genericDefaultTuple.invoke(etsTuple);

    return res.getProperty(0).toNumber() == etsTuple[0]
        && res.getProperty(1).toNumber() == etsTuple[1];
}

function checkGenericDefaultLiteral(): boolean {
    return genericDefaultLiteral.invoke(etsNumber).toNumber() == etsNumber;
}

function checkGenericDefaultIntCallFromTs(): boolean {
    return genericDefaultIntCallFromTs.invoke().toNumber() == etsNumber;
}

function checkGenericDefaultStringCallFromTs(): boolean {
    return genericDefaultStringCallFromTs.invoke().toString() == etsString.toString();
}

function checkGenericDefaultBoolCallFromTs(): boolean {
    return genericDefaultBoolCallFromTs.invoke().toBoolean() == etsBool.toBoolean();
}

function checkGenericDefaultArrCallFromTs(): boolean {
    const res = genericDefaultArrCallFromTs.invoke();

    return res.getProperty(0).toNumber() == etsArr[0];
}

function checkGenericDefaultObjCallFromTs(): boolean {
    return genericDefaultObjCallFromTs.invoke(etsObj).typeOf() == 'object';
}

function checkGenericDefaultUnionCallFromTs(): boolean {
    return genericDefaultUnionCallFromTs.invoke().toNumber() == etsNumber;
}

function checkGenericDefaultTupleCallFromTs(): boolean {
    const res = genericDefaultTupleCallFromTs.invoke();

    return res.getProperty(0).toNumber() == etsTuple[0]
        && res.getProperty(1).toBoolean() == etsTuple[1];
}

function checkGenericDefaultLiteralCallFromTs(): boolean {
    return genericDefaultLiteralCallFromTs.invoke().toNumber() == etsNumber;
}
//generic extend class
function checkGenericExtendClass(): boolean {
    const data = DataClass.instantiate();
    return genericExtendClass.invoke(data).getPropertySafe('data').toString() ==
           data.getPropertySafe('data').toString();
}
