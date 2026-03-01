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

let module = ESValue.load('main_js')
let GenericExtends = module.getProperty('GenericExtends');
let genericClassExtendsCallFromTs = module.getProperty('genericClassExtendsCallFromTs');

function checkClassExtendsInt(): boolean {
    try {
        const tsInt = ESValue.wrapInt(1);
        const res = GenericExtends.invokeMethod('get', tsInt);
        return res.toNumber() === 1;
    } catch (e) {
        return false;
    }
}

function checkClassExtendsString(): boolean {
    try {
        const tsString = ESValue.wrapString('test');
        const res = GenericExtends.invokeMethod('get', tsString);
        return res.toString() === 'test';
    } catch (e) {
        return false;
    }
}

function checkClassExtendsBool(): boolean {
    try {
        const tsBool = ESValue.wrapBoolean(true);
        const res = GenericExtends.invokeMethod('get', tsBool);
        return res.toBoolean() === true;
    } catch (e) {
        return false;
    }
}

function checkClassExtendsArr(): boolean {
    try {
        const jsArray = [1, 2, 3];
        const tsArr = ESValue.wrap(jsArray);
        const res = GenericExtends.invokeMethod('get', tsArr);
        const firstElement = res.getProperty(0);
        return firstElement.toNumber() === 1;
    } catch (e) {
        return false;
    }
}

function checkClassExtendsMethodCallFromTsInt(): boolean {
    try {
        const tsInt = ESValue.wrapInt(1);
        const res = genericClassExtendsCallFromTs.invoke(tsInt);
        return res.toNumber() === 1;
    } catch (e) {
        return false;
    }
}

function checkClassExtendsMethodCallFromTsString(): boolean {
    try {
        const tsString = ESValue.wrapString('test');
        const res = genericClassExtendsCallFromTs.invoke(tsString);
        return res.toString() === 'test';
    } catch (e) {
        return false;
    }
}

function checkClassExtendsMethodCallFromTsBool(): boolean {
    try {
        const tsBool = ESValue.wrapBoolean(true);
        const res = genericClassExtendsCallFromTs.invoke(tsBool);
        return res.toBoolean() === true;
    } catch (e) {
        return false;
    }
}

function checkClassExtendsMethodCallFromTsArr(): boolean {
    try {
        const jsArray = [1, 2, 3];
        const tsArr = ESValue.wrap(jsArray);
        const res = genericClassExtendsCallFromTs.invoke(tsArr);     
        const firstElement = res.getProperty(0);
        return firstElement.toNumber() === 1;
    } catch (e) {
        return false;
    }
}