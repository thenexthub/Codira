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

let module = ESValue.load('to_string');
let etsIdentity = module.getProperty('Identity');
let etsIntValue = module.getProperty('IntValue');
let etsOptional = module.getProperty('Optional');
let etsCurry = module.getProperty('Curry');
let etsCurryToString = module.getProperty('CurryToString');
let etsStringifyValue = module.getProperty('StringifyValue');
let etsThrowing = module.getProperty('Throwing');
let etsNullVal = module.getProperty('NullVal');
let etsNanVal = module.getProperty('NanVal');
let etsUndefinedVal = module.getProperty('UndefinedVal');

function vstringify(val: Any): String {
    return etsStringifyValue.invoke(ESValue.wrap(val)).toString();
}

function castToObject(x: Object): Object {
    return x;
}

class A {
    toString(): String {
        return etsCurryToString.invoke(123).invokeMethod('toString').toString() + 'A';
    };
}

function etsJsvalueToStr() {
    let x: Any = etsIdentity.invoke(new A()).invokeMethod('toString').toString();
    return x + 'A';
}

function etsObjectToStr() {
    let x: Any = etsIdentity.invoke(new A()).invokeMethod('toString').toString();
    let y = castToObject(x).toString();
    return y + 'A';
}

function jsvalObjectToStr(): String {
    let x: Any = etsCurry.invoke(123).invokeMethod('toString');
    try {
        let z: String = x.toString();
        arktest.assertTrue(false);
    } catch (z) {
        if (z instanceof TypeError) {
        let y = castToObject(x.toNumber());
        return y + 'A';
        }
    }
    return 'Unreachable';
}

function implicitToString(): String {
    let x = etsOptional.invoke(false).toString();
    arktest.assertTrue(vstringify(x) === 'string:optionalVal');
    return x + 'A';
}

function testFunctional1(): String {
    return etsCurry.invoke('Str1').invokeMethod('toString').toString() + 'A';
}

function testFunctional2(): String {
    return etsCurryToString.invoke(123).invokeMethod('toString').toString() + 'A';
}

function testNanCall(): String {
    return etsCurry.invoke(NaN).invokeMethod('toString').toNumber() + 'A';
}

function testThrowing(): String {
    try {
        return etsThrowing.invoke().invokeMethod('toString') + 'A';
    } catch (e) {
        if (e instanceof ESError) {
            return e.getValue().toNumber() + 'A';
        } else {
            return e.message;
        }
    }
}

function concatNullToString(): String {
    let x = etsNullVal.invoke().toNull();
    return x + 'A';
}

function concatUndefinedToString(): String {
    let x = etsUndefinedVal.invoke().toUndefined();
    let y = x + 'A';
    return y;
}

function concatNanToString(): String {
    let x = etsNanVal.invoke();
    let y = x.toNumber() + 'A';
    return y;
}

function explicitOptionalToString(): String {
    try {
        return etsOptional.invoke(true).toString() + 'A';
    } catch (e) {
        if (e instanceof NullPointerError) {
            return 'NPE';
        } else {
            return e.message;
        }
    }
}