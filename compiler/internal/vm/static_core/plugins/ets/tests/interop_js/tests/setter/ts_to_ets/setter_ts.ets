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

let module = ESValue.load('js/setter')
let clsUnionSetter = module.getProperty('UnionSetter');
let clsInterfaceSetter = module.getProperty('InterfaceSetter');
let clsBaseClass = module.getProperty('BaseClass');
let clsAbstractClass = module.getProperty('AbstractClass');
let clsSubsetRefSet = module.getProperty('SubsetRefSet');
let clsSubsetValueSet = module.getProperty('SubsetValueSet');
let unionSetterObject = module.getProperty('UnionSetterObject');
let interfaceSetterObject = module.getProperty('InterfaceSetterObject');
let tupleSetObject = module.getProperty('TupleSetObject');
let setterAnyTypeObject = module.getProperty('SetterAnyTypeObject');
let baseClassObject = module.getProperty('BaseClassObject');
let subsetRefSetObject = module.getProperty('SubsetRefSetObject');
let subsetValueSetObject = module.getProperty('SubsetValueSetObject');
let tsTestString = module.getProperty('tsTestString');
let abstractClassObject = module.getProperty('AbstractClassObject');

type TupleT = [int, string];

const strContent = 'ets_test_string';
const numContent = 1;
const tupleContent: TupleT = [numContent, strContent];

function errLog(op: String) {
    console.println("Bug operation: " + op);
}

function checkAbstractClassSetter(): boolean {
    const abstractClass = clsAbstractClass.instantiate();
    abstractClass.setProperty('value', strContent);

    return strContent == abstractClass.getProperty('value').toString();
}

function checkUnionSetter(): boolean {
    const unionSetter = clsUnionSetter.instantiate();
    unionSetter.setProperty('value', numContent);

    return numContent == unionSetter.getProperty('value').toNumber().toInt();
}

function checkInterfaceSetter(): boolean {
    const interfaceSetter = clsInterfaceSetter.instantiate();
    interfaceSetter.setProperty('value', strContent);

    return strContent == interfaceSetter.getProperty('value').toString();
}

function checkUserClassSetter() {
    const base = clsBaseClass.instantiate();
    base.setProperty('value', strContent);

    return strContent == base.getProperty('value').toString();
}

function checkExtendsClass() {
    const subsetRefSet = clsSubsetRefSet.instantiate();
    subsetRefSet.setProperty('value', strContent);

    return strContent == subsetRefSet.getProperty('value').toString();
}

function checkExtendsClassWithValue() {
    const subsetValueSet = clsSubsetValueSet.instantiate();
    subsetValueSet.setProperty('value', strContent);

    return strContent == subsetValueSet.getProperty('value').toString();
}

function checkTupleTypeObjectFormTs() {
    tupleSetObject.setProperty('value', tupleContent);

    if (tupleSetObject.getProperty('value').getProperty(0).toNumber() == tupleContent[0] &&
        tupleSetObject.getProperty('value').getProperty(1).toString() == tupleContent[1]) {
        return true;
    } else {
        errLog('tuple type object setter not work.');

        return false;
    }
}

function checkSetterAnyTypeObjectFormTs() {
    setterAnyTypeObject.setProperty('value', strContent);

    return check(setterAnyTypeObject.getProperty('value').toString(), strContent, 'setter any type object not work.');
}

function checkSetterUnionTypeObjectFormTs() {
    unionSetterObject.setProperty('value', numContent);

    return check(unionSetterObject.getProperty('value').toNumber().toInt(), numContent, 'setter union type object not work.');
}

function checkSetterInterfaceObjectFormTs() {
    interfaceSetterObject.setProperty('value', strContent);

    return check(interfaceSetterObject.getProperty('value').toString(), strContent, 'setter interface object not work.');
}

function checkSetterUserObjectFormTs() {
    baseClassObject.setProperty('value', strContent);

    return check(baseClassObject.getProperty('value').toString(), strContent, 'setter user object not work.');
}

function checkSetterSubsetRefSetObjectFormTs() {
    subsetRefSetObject.setProperty('value', strContent);

    return check(subsetRefSetObject.getProperty('value').toString(), strContent, 'setter subset ref set object not work.');
}

function checkSetterSubsetValueSetObjectFormTs() {
    if (!ESValue.areEqualSafe(subsetValueSetObject.getProperty('value'), tsTestString)) {
        errLog('incorect value in object.');

        return false;
    }

    subsetValueSetObject.setProperty('value', strContent);

    return check(subsetValueSetObject.getProperty('value').toString(), strContent, 'setter subset value set object not work.');
}

function checkSetterAbstractClassObjectFormTs() {
    abstractClassObject.setProperty('value', strContent);

    return check(abstractClassObject.getProperty('value').toString(), strContent, 'setter abstract class object not work.');
}

function check(value: string | int, content: string | int, message: string): boolean {
    if (value == content) {
        return true;
    } else {
        errLog(message);

        return false;
    }
}
