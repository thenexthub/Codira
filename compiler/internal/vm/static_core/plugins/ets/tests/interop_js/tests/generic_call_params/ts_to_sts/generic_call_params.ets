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

let module = ESObject.load('../../plugins/ets/tests/interop_js/tests/generic_call_params/ts_to_sts/generic_call_params');

let applyFunctionGeneric = module.getProperty('applyFunctionGeneric');
let applyFunctionGenericTuple = module.getProperty('applyFunctionGenericTuple');
let genericExtendsClass = module.getProperty('genericExtendsClass');
let Vector = module.getProperty('Vector');
let genericExtendsClassCallFromTs = module.getProperty('genericExtendsClassCallFromTs');
let applyFunctionGenericArray = module.getProperty('applyFunctionGenericArray');
let applyFunctionGenericArrayCallFromTs = module.getProperty('applyFunctionGenericArrayCallFromTs');
let applyFunWithConstraints = module.getProperty('applyFunWithConstraints');
let genericKeyof = module.getProperty('genericKeyof');
let ClassSubset = module.getProperty('ClassSubset');
let subsetClassCallFromTs = module.getProperty('subsetClassCallFromTs');
let applyFunctionGenericUnion = module.getProperty('applyFunctionGenericUnion');

type TupleT = [string, int];
const etsInt = 1;
const etsUnion: int = etsInt;
const etsString: string = 'string';
const arrForLoop: Array<int> = [0, 1, 2]
const etsTuple: Array<string|int> = [etsString, etsInt];

class EtsUser {
    age = etsInt;
    data = etsString;
}
const etsUser = new EtsUser();
const unionFoo = (a: int, b: int): Array<int> => [a, b];
const add = (a: int, b: int): int => a + b;
const concat = (a: string, b: string): string => a + ' ' + b;
const push = (a: string, b: int): Array<string|int> => [a, b];

function checkApplyFunctionGenericString(): boolean {
    return applyFunctionGeneric.invoke(etsString, etsString, concat).toString() == etsString + ' ' + etsString;
}

function checkApplyFunctionGenericInt(): boolean {
    return applyFunctionGeneric.invoke(etsInt, etsInt, add).toNumber() == etsInt + etsInt;
}

function checkApplyFunctionGenericTuple(): boolean {
    const res = applyFunctionGenericTuple.invoke(etsTuple[0], etsTuple[1], push);
    return res.getProperty(0).toString() == etsTuple[0]
           && res.getProperty(1).toNumber() == etsTuple[1];
}

function checkGenericExtendsClass(): boolean {
    const res = genericExtendsClass.invoke(Vector.instantiate(etsInt));
    return res.invokeMethod('get').toNumber() > etsInt;
}

function checkGenericExtendsClassCallFromTs(): boolean {
    const res = genericExtendsClassCallFromTs.invoke(Vector.instantiate(etsInt));
    return res.invokeMethod('get').toNumber() > etsInt;
}

function checkApplyFunctionGenericArrayCallFromTs() {
    const res = arrSum(arrForLoop);
    return applyFunctionGenericArrayCallFromTs.invoke(arrForLoop).toNumber() == res;
}

function checkApplyFunctionGenericArray(): boolean {
    const res = arrSum(arrForLoop);
    return applyFunctionGenericArray.invoke(arrForLoop, add).toNumber() == res;
}

function arrSum(arr: Array<int>): int {
    let res = 0
    for (let i = 0; i < arrForLoop.length; ++i) {
        res += arrForLoop[i];
    }
    return res;
}

const toStringArrowFu = (a: int, b: string): string => a + ' ' + b;
function toStringFu(a: int, b: string): string {
    return a + ' ' + b;
}

//NOTE: issue (18005) fix function as param
// function checkApplyFunWithConstraintsFun() {
// const res = applyFunWithConstraints(etsInt, etsString, toStringFu);
// return res as string == etsInt + ' ' + etsString as string;
// }

function checkApplyFunWithConstraintsArrowFun() {
    const res = applyFunWithConstraints.invoke(etsInt, etsString, toStringArrowFu);
    return res.toString() == etsInt + ' ' + etsString;
}

function checkGenericKeyof(): boolean {
    const keys: Array<string> = ['age', 'data'];
    const res = genericKeyof.invoke(etsUser, keys);
    const checkKeys = res.getProperty('keys').getProperty(0).toString() == keys[0] &&
                      res.getProperty('keys').getProperty(1).toString() == keys[1];
    const checkObj = res.getProperty('obj').getProperty('age').toNumber() == etsInt && 
                     res.getProperty('obj').getProperty('data').toString() == etsString
    return checkKeys && checkObj;
}

function checkClassSubset(): boolean {
    const GClass = ClassSubset.invoke();
    return GClass.getProperty('get').invoke(etsInt, etsInt, add).toNumber() > etsInt;
}

function checkSubsetClassCallFromTs(): boolean {
    return subsetClassCallFromTs.invoke().toNumber() > etsInt;
}

function checkApplyFunctionGenericUnion(): boolean {
    const res = applyFunctionGenericUnion.invoke(etsUnion, etsUnion, unionFoo);
    return res.getProperty(0).toNumber() == etsUnion &&
           res.getProperty(1).toNumber() == etsUnion;
}