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

let JSCallee = require('./callee.js');

let actualCallee = {
    PassVoid : JSCallee.Pass,
    PassArrays : JSCallee.Pass,
    PassNumbers : JSCallee.Pass,
    PassStrings : JSCallee.Pass,
    GetArray : JSCallee.GetArray,
    GetString : JSCallee.GetString,
    GetObject : JSCallee.GetObject,
    GetNumber : JSCallee.GetNumber
};

module.exports.initBenchmarks = function initBenchmarks(arkVM)
{
    let benchmarks = {
        voidPasser : voidPasser,
        numPasser : numPasser,
        strPasser : strPasser,
        arrPasser : arrPasser,
        arrGetter : arrGetter,
        strGetter : strGetter,
        objGetter : objGetter,
        numGetter : numGetter
    };

    if (arkVM) {
        actualCallee = arkVM.getClass('Lcallee/STSCallee;');
    }

    return benchmarks;
};

function voidPasser(iters) {
    let PassVoid = actualCallee.PassVoid;
    for (let i = 0; i < iters; ++i) {
        PassVoid();
    }
}

function strPasser(iters) {
    let PassStrings = actualCallee.PassStrings;
    let u0 = 'foo0';
    let u1 = 'foo1';
    let u2 = 'foo2';
    let u3 = 'foo3';
    let u4 = 'foo4';
    let u5 = 'foo5';
    let u6 = 'foo6';
    let u7 = 'foo7';
    let u8 = 'foo8';
    let u9 = 'foo9';
    for (let i = 0; i < iters; ++i) {
        PassStrings(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function numPasser(iters) {
    let PassNumbers = actualCallee.PassNumbers;
    let u0 = 0.0;
    let u1 = 0.1;
    let u2 = 0.2;
    let u3 = 0.3;
    let u4 = 0.4;
    let u5 = 0.5;
    let u6 = 0.6;
    let u7 = 0.7;
    let u8 = 0.8;
    let u9 = 0.9;
    for (let i = 0; i < iters; ++i) {
        PassNumbers(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function arrPasser(iters) {
    let PassArrays = actualCallee.PassArrays;
    let u0 = [0.0];
    let u1 = [0.1];
    let u2 = [0.2];
    let u3 = [0.3];
    let u4 = [0.4];
    let u5 = [0.5];
    let u6 = [0.6];
    let u7 = [0.7];
    let u8 = [0.8];
    let u9 = [0.9];
    for (let i = 0; i < iters; ++i) {
        PassArrays(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function arrGetter(iters) {
    let GetArray = actualCallee.GetArray;
    for (let i = 0; i < iters; ++i) {
        let r = GetArray();
    }
}

function strGetter(iters) {
    let GetString = actualCallee.GetString;
    for (let i = 0; i < iters; ++i) {
        let r = GetString();
    }
}

function numGetter(iters) {
    let GetNumber = actualCallee.GetNumber;
    for (let i = 0; i < iters; ++i) {
        let r = GetNumber();
    }
}

function objGetter(iters) {
    let GetObject = actualCallee.GetObject;
    for (let i = 0; i < iters; ++i) {
        let r = GetObject();
    }
}
