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

let tsString = module.getProperty('tsString');
let tsNumber = module.getProperty('tsNumber');
let PublicGetterClass = module.getProperty('PublicGetterClass');
let createPublicGetterClassFromTs = module.getProperty('createPublicGetterClassFromTs');
let publicGetterInstanceClass = module.getProperty('publicGetterInstanceClass');
let UnionTypeClass = module.getProperty('UnionTypeClass');
let createUnionTypeGetterClassFromTs = module.getProperty('createUnionTypeGetterClassFromTs');
let unionTypeGetterInstanceClassInt = module.getProperty('unionTypeGetterInstanceClassInt');
let unionTypeGetterInstanceClassString = module.getProperty('unionTypeGetterInstanceClassString');
let LiteralClass = module.getProperty('LiteralClass');
let createLiteralTypeGetterClassFromTs = module.getProperty('createLiteralTypeGetterClassFromTs');
let literalTypeGetterInstanceClassInt = module.getProperty('literalTypeGetterInstanceClassInt');
let literalTypeGetterInstanceClassString = module.getProperty('literalTypeGetterInstanceClassString');
let TupleTypeClass = module.getProperty('TupleTypeClass');
let createTupleTypeGetterClassFromTs = module.getProperty('createTupleTypeGetterClassFromTs');
let tupleTypeGetterInstanceClass = module.getProperty('tupleTypeGetterInstanceClass');
let AnyTypeClass = module.getProperty('AnyTypeClass');
let createAnyTypeGetterClassFromTs = module.getProperty('createAnyTypeGetterClassFromTs');
let anyTypeGetterInstanceClass = module.getProperty('anyTypeGetterInstanceClass');
let anyTypeExplicitGetterInstanceClass = module.getProperty('anyTypeExplicitGetterInstanceClass');
let SubsetByRef = module.getProperty('SubsetByRef');
let createSubsetByRefGetterClassFromTs = module.getProperty('createSubsetByRefGetterClassFromTs');
let subsetByRefInstanceClass = module.getProperty('subsetByRefInstanceClass');
let SubsetByValueClass = module.getProperty('SubsetByValueClass');
let createSubsetByValueGetterClassFromTs = module.getProperty('createSubsetByValueGetterClassFromTs');
let subsetByValueInstanceClass = module.getProperty('subsetByValueInstanceClass');

const etsNumber: int = 1;
const etsString: string = 'string';
const etsBool: boolean = true;
const etsObj: Object = new Object();
const etsUnion: int = etsNumber;

function checkGetterPublicClass(): boolean {
    try {
        const GClass = PublicGetterClass.instantiate();
        const value = GClass.getProperty('value');
        return value.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkCreatePublicGetterClassFromTs(): boolean {
    try {
        const GClass = createPublicGetterClassFromTs.invoke();
        const value = GClass.getProperty('value');
        return tsString.toString() == value.toString();
    } catch (e) {
        return false;
    }
}

function checkPublicGetterInstanceClassFromTs(): boolean {
    try {
        const value = publicGetterInstanceClass.getProperty('value');
        return tsString.toString() == value.toString();
    } catch (e) {
        return false;
    }
}

function checkUnionTypeGetterClassInt(): boolean {
    try {
        const GClass = UnionTypeClass.instantiate(tsNumber);
        const value = GClass.getProperty('value');
        return value.toNumber() == tsNumber.toNumber();
    } catch (e) {
        return false;
    }
}

function checkCreateUnionTypeGetterClassFromTsInt(): boolean {
    try {
        const GClass = createUnionTypeGetterClassFromTs.invoke(tsNumber);
        const value = GClass.getProperty('value');
        return value.toNumber() == tsNumber.toNumber();
    } catch (e) {
        return false;
    }
}

function checkInstanceUnionTypeGetterClassFromTsInt(): boolean {
    try {
        const value = unionTypeGetterInstanceClassInt.getProperty('value');
        return tsNumber.toNumber() == value.toNumber();
    } catch (e) {
        return false;
    }
}

function checkUnionTypeGetterClassString(): boolean {
    try {
        const GClass = UnionTypeClass.instantiate(tsString);
        const value = GClass.getProperty('value');
        return value.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkCreateUnionTypeGetterClassFromTsString(): boolean {
    try {
        const GClass = createUnionTypeGetterClassFromTs.invoke(tsString);
        const value = GClass.getProperty('value');
        return value.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkInstanceUnionTypeGetterClassFromTsString(): boolean {
    try {
        const value = unionTypeGetterInstanceClassString.getProperty('value');
        return tsString.toString() == value.toString();
    } catch (e) {
        return false;
    }
}

function checkLiteralTypeGetterClassInt(): boolean {
    try {
        const GClass = LiteralClass.instantiate(tsNumber);
        const value = GClass.getProperty('value');
        return value.toNumber() == tsNumber.toNumber();
    } catch (e) {
        return false;
    }
}

function checkLiteralTypeGetterClassString(): boolean {
    try {
        const GClass = LiteralClass.instantiate(tsString);
        const value = GClass.getProperty('value');
        return value.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkCreateLiteralTypeGetterClassFromTsInt(): boolean {
    try {
        const GClass = createLiteralTypeGetterClassFromTs.invoke(tsNumber);
        const value = GClass.getProperty('value');
        return value.toNumber() == tsNumber.toNumber();
    } catch (e) {
        return false;
    }
}

function checkCreateLiteralTypeGetterClassFromTsString(): boolean {
    try {
        const GClass = createLiteralTypeGetterClassFromTs.invoke(tsString);
        const value = GClass.getProperty('value');
        return value.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkInstanceLiteralTypeGetterClassFromTsInt(): boolean {
    try {
        const value = literalTypeGetterInstanceClassInt.getProperty('value');
        return tsNumber.toNumber() == value.toNumber();
    } catch (e) {
        return false;
    }
}

function checkInstanceLiteralTypeGetterClassFromTsString(): boolean {
    try {
        const value = literalTypeGetterInstanceClassString.getProperty('value');
        return tsString.toString() == value.toString();
    } catch (e) {
        return false;
    }
}

function checkTupleTypeGetterClass(): boolean {
    try {
        const GClass = TupleTypeClass.instantiate([etsNumber, etsString]);
        const value = GClass.getProperty('value');
        const elem0 = value.getProperty(0);
        const elem1 = value.getProperty(1);
        return elem0.toNumber() == tsNumber.toNumber() &&
               elem1.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkCreateTupleTypeGetterClassFromTs(): boolean {
    try {
        const GClass = createTupleTypeGetterClassFromTs.invoke([etsNumber, etsString]);
        const value = GClass.getProperty('value');
        const elem0 = value.getProperty(0);
        const elem1 = value.getProperty(1);
        return elem0.toNumber() == tsNumber.toNumber() &&
               elem1.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkInstanceTupleTypeGetterClassFromTs(): boolean {
    try {
        const value = tupleTypeGetterInstanceClass.getProperty('value');
        const elem0 = value.getProperty(0);
        const elem1 = value.getProperty(1);
        return elem0.toNumber() == tsNumber.toNumber() &&
               elem1.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkAnyTypeGetterClassInt(): boolean {
    try {
        const GClass = AnyTypeClass.instantiate();
        GClass.setProperty('_value', ESValue.wrapInt(etsNumber));
        const value = GClass.getProperty('value');
        return value.toNumber() == etsNumber;
    } catch (e) {
        return false;
    }
}

function checkAnyTypeGetterClassString(): boolean {
    try {
        const GClass = AnyTypeClass.instantiate();
        GClass.setProperty('_value', ESValue.wrapString(etsString));
        const value = GClass.getProperty('value');
        return value.toString() == etsString;
    } catch (e) {
        return false;
    }
}

function checkAnyTypeGetterClassBool(): boolean {
    try {
        const GClass = AnyTypeClass.instantiate();
        GClass.setProperty('_value', ESValue.wrapBoolean(etsBool));
        const value = GClass.getProperty('value');
        return value.toBoolean() == etsBool;
    } catch (e) {
        return false;
    }
}

function checkAnyTypeGetterClassArr(): boolean {
    try {
        const GClass = AnyTypeClass.instantiate();
        GClass.setProperty('_value', [ESValue.wrapInt(etsNumber)]);
        const value = GClass.getProperty('value');
        return value != null;
    } catch (e) {
        return false;
    }
}

function checkAnyTypeGetterClassObj(): boolean {
    try {
        const GClass = AnyTypeClass.instantiate();
        GClass.setProperty('_value', etsObj);
        const value = GClass.getProperty('value');
        return value.typeOf() == 'object';
    } catch (e) {
        return false;
    }
}

function checkAnyTypeGetterClassTuple(): boolean {
    try {
        const GClass = AnyTypeClass.instantiate();
        GClass.setProperty('_value', [tsNumber, tsString]);
        const value = GClass.getProperty('value');
        const elem0 = value.getProperty(0);
        const elem1 = value.getProperty(1);
        return elem0.toNumber() == tsNumber.toNumber() &&
               elem1.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkAnyTypeGetterClassUnion(): boolean {
    try {
        const GClass = AnyTypeClass.instantiate();
        GClass.setProperty('_value', ESValue.wrapInt(etsUnion));
        const value = GClass.getProperty('value');
        return value.toNumber() == etsUnion;
    } catch (e) {
        return false;
    }
}

function checkCreateAnyTypeGetterClassFromTsInt(): boolean {
    try {
        const GClass = createAnyTypeGetterClassFromTs.invoke();
        GClass.setProperty('_value', ESValue.wrapInt(etsNumber));
        const value = GClass.getProperty('value');
        return value.toNumber() == etsNumber;
    } catch (e) {
        return false;
    }
}

function checkCreateAnyTypeGetterClassFromTsString(): boolean {
    try {
        const GClass = createAnyTypeGetterClassFromTs.invoke();
        GClass.setProperty('_value', ESValue.wrapString(etsString));
        const value = GClass.getProperty('value');
        return value.toString() == etsString;
    } catch (e) {
        return false;
    }
}

function checkCreateAnyTypeGetterClassFromTsBool(): boolean {
    try {
        const GClass = createAnyTypeGetterClassFromTs.invoke();
        GClass.setProperty('_value', ESValue.wrapBoolean(etsBool));
        const value = GClass.getProperty('value');
        return value.toBoolean() == etsBool;
    } catch (e) {
        return false;
    }
}

function checkCreateAnyTypeGetterClassFromTsArr(): boolean {
    try {
        const GClass = createAnyTypeGetterClassFromTs.invoke();
        GClass.setProperty('_value', [ESValue.wrapInt(etsNumber)]);
        const value = GClass.getProperty('value');
        return value != null;
    } catch (e) {
        return false;
    }
}

function checkCreateAnyTypeGetterClassFromTsObj(): boolean {
    try {
        const GClass = createAnyTypeGetterClassFromTs.invoke();
        GClass.setProperty('_value', etsObj);
        const value = GClass.getProperty('value');
        return value != null;
    } catch (e) {
        return false;
    }
}

function checkCreateAnyTypeGetterClassFromTsTuple(): boolean {
    try {
        const GClass = createAnyTypeGetterClassFromTs.invoke();
        GClass.setProperty('_value', [etsNumber, etsString]);
        const value = GClass.getProperty('value');
        const elem0 = value.getProperty(0);
        const elem1 = value.getProperty(1);
        return elem0.toNumber() == tsNumber.toNumber() &&
               elem1.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkCreateAnyTypeGetterClassFromTsUnion(): boolean {
    try {
        const GClass = createAnyTypeGetterClassFromTs.invoke();
        GClass.setProperty('_value', ESValue.wrapInt(etsUnion));
        const value = GClass.getProperty('value');
        return value.toNumber() == etsUnion;
    } catch (e) {
        return false;
    }
}

function checkInstanceAnyTypeGetterClassFromTsInt(): boolean {
    try {
        anyTypeGetterInstanceClass.setProperty('_value', ESValue.wrapInt(etsNumber));
        const value = anyTypeGetterInstanceClass.getProperty('value');
        return value.toNumber() == etsNumber;
    } catch (e) {
        return false;
    }
}

function checkInstanceAnyTypeGetterClassFromTsString(): boolean {
    try {
        anyTypeGetterInstanceClass.setProperty('_value', ESValue.wrapString(etsString));
        const value = anyTypeGetterInstanceClass.getProperty('value');
        return value.toString() == etsString;
    } catch (e) {
        return false;
    }
}

function checkInstanceAnyTypeGetterClassFromTsBool(): boolean {
    try {
        anyTypeGetterInstanceClass.setProperty('_value', ESValue.wrapBoolean(etsBool));
        const value = anyTypeGetterInstanceClass.getProperty('value');
        return value.toBoolean() == etsBool;
    } catch (e) {
        return false;
    }
}

function checkInstanceAnyTypeGetterClassFromTsArr(): boolean {
    try {
        anyTypeGetterInstanceClass.setProperty('_value', [ESValue.wrapInt(etsNumber)]);
        const value = anyTypeGetterInstanceClass.getProperty('value');
        return value != null;
    } catch (e) {
        return false;
    }
}

function checkInstanceAnyTypeGetterClassFromTsObj(): boolean {
    try {
        anyTypeGetterInstanceClass.setProperty('_value', etsObj);
        const value = anyTypeGetterInstanceClass.getProperty('value');
        return value != null;
    } catch (e) {
        return false;
    }
}

function checkInstanceAnyTypeGetterClassFromTsTuple(): boolean {
    try {
        anyTypeGetterInstanceClass.setProperty('_value', [etsNumber, etsString]);
        const value = anyTypeGetterInstanceClass.getProperty('value');
        const elem0 = value.getProperty(0);
        const elem1 = value.getProperty(1);
        return elem0.toNumber() == tsNumber.toNumber() &&
               elem1.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkInstanceAnyTypeGetterClassFromTsUnion(): boolean {
    try {
        anyTypeGetterInstanceClass.setProperty('_value', ESValue.wrapInt(etsUnion));
        const value = anyTypeGetterInstanceClass.getProperty('value');
        return value.toNumber() == etsUnion;
    } catch (e) {
        return false;
    }
}

function checkInstanceAnyExplicitTypeGetterClassFromTsExplicit(): boolean {
    try {
        anyTypeExplicitGetterInstanceClass.setProperty('_value', ESValue.wrapString(etsString));
        const value = anyTypeExplicitGetterInstanceClass.getProperty('value');
        return value.toString() == etsString;
    } catch (e) {
        return false;
    }
}

function checkGetterSubsetByRefClass(): boolean {
    try {
        const GClass = SubsetByRef.instantiate();
        const value = GClass.getProperty('value');
        return value.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkCreateSubsetByRefGetterClassFromTs(): boolean {
    try {
        const GClass = createSubsetByRefGetterClassFromTs.invoke();
        const value = GClass.getProperty('value');
        return tsString.toString() == value.toString();
    } catch (e) {
        return false;
    }
}

function checkInstanceSubsetByRefGetterClassFromTs(): boolean {
    try {
        const value = subsetByRefInstanceClass.getProperty('value');
        return value.toString() == etsString;
    } catch (e) {
        return false;
    }
}

function checkGetterSubsetByValueClass(): boolean {
    try {
        const publicGetter = PublicGetterClass.instantiate();
        const publicValue = publicGetter.getProperty('value');
        const GClass = SubsetByValueClass.instantiate(publicValue);
        const value = GClass.getProperty('value');
        return value.toString() == tsString.toString();
    } catch (e) {
        return false;
    }
}

function checkCreateSubsetByValueGetterClassFromTs(): boolean {
    try {
        const GClass = createSubsetByValueGetterClassFromTs.invoke();
        const value = GClass.getProperty('value');
        return tsString.toString() == value.toString();
    } catch (e) {
        return false;
    }
}

function checkInstanceSubsetByValueGetterClassFromTs(): boolean {
    try {
        const value = subsetByValueInstanceClass.getProperty('value');
        return value.toString() == etsString;
    } catch (e) {
        return false;
    }
}