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

let module = ESValue.load('esvaluewithstringkey_js')
let etsA = module.getProperty('A');

function checkFieldAccessByDot(): int {
    let e: ESValue = etsA.instantiate();
    let res = e.getProperty('Ekko').toString();
    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithStringLiteral(): int {
    let e: ESValue = etsA.instantiate();
    let res = e.getProperty('Ekko').toString();
    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithStringKey() : int {
    let e: ESValue = etsA.instantiate();
    let stringKey: string = 'Ekko';
    let res = e.getProperty(stringKey).toString();
    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function foo(es: ESValue, str: string): string {
    return es.getProperty(str).toString();
}

function checkEsvalueWithStringFromFoo() : int {
    let es: ESValue = etsA.instantiate();
    let stringKey: string = 'Ekko';
    let res: string = foo(es, stringKey);

    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithExpression(): int {
    let es: ESValue = etsA.instantiate();
    let stringKey: string = 'Ekk';
    let res = es.getProperty(stringKey + 'o').toString();

    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithExpressionStrLiterals(): int {
    let es: ESValue = etsA.instantiate();
    let res = es.getProperty('Ekk' + 'o').toString();

    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithExpressionStrings(): int {
    let es: ESValue = etsA.instantiate();
    let stringKey1: string = 'Ekk';
    let stringKey2: string = 'o';
    let res = es.getProperty(stringKey1 + stringKey2).toString();

    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}
