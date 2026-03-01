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

let module = ESValue.load('js_code');
let BOOLEAN_VALUE = module.getProperty('BOOLEAN_VALUE');
let ENUM_VALUE = module.getProperty('ENUM_VALUE');
let GenericInterface = module.getProperty('GenericInterface');
let STRING_VALUE = module.getProperty('STRING_VALUE');
let TFunctionReturnsAny = module.getProperty('TFunctionReturnsAny');
let TFunctionReturnsBigInt = module.getProperty('TFunctionReturnsBigInt');
let TFunctionReturnsBoolean = module.getProperty('TFunctionReturnsBoolean');
let TFunctionReturnsEnum = module.getProperty('TFunctionReturnsEnum');
let TFunctionReturnsFunctionOfType = module.getProperty('TFunctionReturnsFunctionOfType');
let TFunctionReturnsNull = module.getProperty('TFunctionReturnsNull');
let TFunctionReturnsNumber = module.getProperty('TFunctionReturnsNumber');
let TFunctionReturnsString = module.getProperty('TFunctionReturnsString');
let TFunctionReturnsUndefined = module.getProperty('TFunctionReturnsUndefined ');
let UNDEFINED = module.getProperty('UNDEFINED');
let genericInterfaceImplementation = module.getProperty('genericInterfaceImplementation');


/* Note: nullish \ undefish types here are workarounds for note 17745*/
type UndefishHelperType = undefined | string | number;
type NullishHelperType = null | string | number

function cleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let fullGcId = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(fullGcId);
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
}

function type_imported__returnAny(){
    const anyGetter: ESValue = genericInterfaceImplementation.getProperty('getAny') as ESValue;
    return anyGetter.invoke().typeOf() == 'object';
}

function type_imported__returnString(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getString') as ESValue;
    console.log(getterFn.invoke().typeOf());
    console.log(getterFn.invoke().toString());
    return getterFn.invoke().typeOf() == 'string' && (getterFn.invoke().toString() == 'Panda')
}

function type_imported__returnBoolean(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getBoolean') as ESValue;
    return getterFn.invoke().typeOf() == 'boolean' &&
           (getterFn.invoke().toBoolean()) == true &&
           (getterFn.invoke().toBoolean()) != false;
}

// NOTE: this test uses JS integer value which is longer than 32 bits
function type_imported__returnInteger(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getInt') as ESValue;
    return getterFn.invoke().typeOf() == 'number' && (getterFn.invoke().toNumber().toInt()) > 0 &&
           (getterFn.invoke().toNumber().toInt()) / 2 == Double.toInt((getterFn.invoke().toNumber().toInt()) * 0.5);
}

// NOTE: this test uses JS integer value which is longer than 32 bit
function type_imported__returnNegativeInteger(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getNegativeInt') as ESValue;
    return getterFn.invoke().typeOf() == 'number' && (getterFn.invoke().toNumber().toLong()) < 0 && (getterFn.invoke().toNumber().toLong()) * -1 > 0;
}


function type_imported__returnInfinity(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getInfinity');
    console.log(getterFn.invoke().typeOf());
    console.log(getterFn.invoke().toNumber());
    console.log((getterFn.invoke().toNumber()) - 1 == getterFn.invoke());
    console.log(!Number.isFinite(getterFn.invoke().toNumber()));
    const isValid: boolean = (getterFn.invoke().typeOf() == 'number') &&
        ((getterFn.invoke().toNumber()) > 0) &&
        ((getterFn.invoke().toNumber()) - 1 == getterFn.invoke().toNumber()) &&
        !Number.isFinite(getterFn.invoke().toNumber());
    cleanup();
    return isValid;
}

function type_imported__returnNegativeInfinity(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getNegativeInfinity');
    const isValid: boolean = (getterFn.invoke().typeOf() == 'number') &&
        ((getterFn.invoke().toNumber()) < 0) && 
        ((getterFn.invoke().toNumber()) + 1 == getterFn.invoke().toNumber()) && 
        !Number.isFinite(getterFn.invoke().toNumber());
    cleanup();
    return isValid;
}


function type_imported__returnNaN(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getNanAsNumber') as ESValue;
    const isValid: boolean = `${(getterFn.invoke().toNumber())}` == 'NaN' && (Number.isNaN(getterFn.invoke().toNumber()));
    cleanup();
    return isValid;
}

function type_imported__returnBigint(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getBigInt');
    return getterFn.invoke().typeOf() == 'bigint';
}

function type_imported__returnEnum(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getEnum');
    const returnValue = getterFn.invoke();
    const isAbleToGetExistingProperties = ((returnValue.getProperty('OPTION_ONE').toNumber().toInt() == 0) && 
                                            (returnValue.getProperty('OPTION_TWO').toNumber() == 1) && 
                                            (returnValue.getProperty('OPTION_THREE').toNumber() == 2));
    const isReturningUndefForMissingProperties = returnValue.getProperty('OPTION_FOUR').unwrap() as UndefishHelperType == undefined;
    cleanup();
    return getterFn.invoke().typeOf() == "object" && isAbleToGetExistingProperties && isReturningUndefForMissingProperties
}

function type_imported__returnUndefined(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getUndefined');
    const returnValue = getterFn.invoke().unwrap() as UndefishHelperType;
    cleanup();
    return returnValue == undefined && returnValue != '';
}

function type_imported__returnNull(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getNull');
    const returnValue = getterFn.invoke().unwrap() as NullishHelperType;
    return returnValue == null;
}


function type_imported__returnFunction(){
    const getterFn: ESValue = genericInterfaceImplementation.getProperty('getFunctionReturningType');
    const returnStringValue = getterFn.invoke('panda');
    const returnNumericValue = getterFn.invoke(1);
    return returnStringValue.invoke().toString() == 'pandaPANDA' &&
        (returnNumericValue.invoke().toNumber().toFloat())> 3 &&
        (returnNumericValue.invoke().toNumber().toFloat()) < 4;
}
