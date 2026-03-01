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

import { STSCallee } from "./callee.ets";


class jsvars {
    static global = JSRuntime.getGlobal();
    static jsCallee = JSRuntime.getPropertyJSValue(jsvars.global, "jsCallee");
};

class $jscall {
    static Init = JSRuntime.__initJSCallClass();

    native static void$(callee: JSValue, self: JSValue): void;
    native static void$10s(callee: JSValue, self: JSValue, u0: String, u1: String, u2: String, u3: String, u4: String, u5: String, u6: String, u7: String, u8: String, u9: String): void;
    native static void$10d(callee: JSValue, self: JSValue, u0: double, u1: double, u2: double, u3: double, u4: double, u5: double, u6: double, u7: double, u8: double, u9: double): void;
    native static void$10a(callee: JSValue, self: JSValue, u0: FixedArray<double>, u1: FixedArray<double>, u2: FixedArray<double>, u3: FixedArray<double>, u4: FixedArray<double>, u5: FixedArray<double>, u6: FixedArray<double>, u7: FixedArray<double>, u8: FixedArray<double>, u9: FixedArray<double>): void;

    native static str$(callee: JSValue, self: JSValue): string;
    native static arr$(callee: JSValue, self: JSValue): FixedArray<double>;
    native static any$(callee: JSValue, self: JSValue): JSValue;
    native static num$(callee: JSValue, self: JSValue): double;
};

class $jsnew {
    static Init = JSRuntime.__initJSNewClass();
};

export function initBenchmarks(isInterop : boolean): Object
{
    class Benchmarks {
        voidPasser = voidPasser
        numPasser = numPasser
        strPasser = strPasser
        arrPasser = arrPasser
        arrGetter = arrGetter
        strGetter = strGetter
        objGetter = objGetter
        numGetter = numGetter
    }

    class InteropBenchmarks {
        voidPasser = interop_voidPasser
        numPasser = interop_numPasser
        strPasser = interop_strPasser
        arrPasser = interop_arrPasser
        arrGetter = interop_arrGetter
        strGetter = interop_strGetter
        objGetter = interop_objGetter
        numGetter = interop_numGetter
        arrAnyGetter = interop_arrAnyGetter
        strAnyGetter = interop_strAnyGetter
        numAnyGetter = interop_numAnyGetter
    }

    if (isInterop) {
        return new InteropBenchmarks;
    }
    return new Benchmarks;
}

function voidPasser(iters: int): void {
    let PassVoid: ()=>void = STSCallee.PassVoid;
    for (let i = 0; i < iters; ++i) {
        PassVoid();
    }
}

function strPasser(iters: int): void {
    let PassStrings: (
        a0: string,
        a1: string,
        a2: string,
        a3: string,
        a4: string,
        a5: string,
        a6: string,
        a7: string,
        a8: string,
        a9: string
    )=>void = STSCallee.PassStrings;

    let u0 = "foo0";
    let u1 = "foo1";
    let u2 = "foo2";
    let u3 = "foo3";
    let u4 = "foo4";
    let u5 = "foo5";
    let u6 = "foo6";
    let u7 = "foo7";
    let u8 = "foo8";
    let u9 = "foo9";
    for (let i = 0; i < iters; ++i) {
        PassStrings(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function numPasser(iters: int): void {
    let PassNumbers: (
        a0: double,
        a1: double,
        a2: double,
        a3: double,
        a4: double,
        a5: double,
        a6: double,
        a7: double,
        a8: double,
        a9: double
    )=>void  = STSCallee.PassNumbers;
    let u0 = .0;
    let u1 = .1;
    let u2 = .2;
    let u3 = .3;
    let u4 = .4;
    let u5 = .5;
    let u6 = .6;
    let u7 = .7;
    let u8 = .8;
    let u9 = .9;
    for (let i = 0; i < iters; ++i) {
        PassNumbers(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function arrPasser(iters: int): void {
    let PassArrays: (
        a0: FixedArray<double>,
        a1: FixedArray<double>,
        a2: FixedArray<double>,
        a3: FixedArray<double>,
        a4: FixedArray<double>,
        a5: FixedArray<double>,
        a6: FixedArray<double>,
        a7: FixedArray<double>,
        a8: FixedArray<double>,
        a9: FixedArray<double>
    )=>void = STSCallee.PassArrays;
    let u0: FixedArray<double> = [.0];
    let u1: FixedArray<double> = [.1];
    let u2: FixedArray<double> = [.2];
    let u3: FixedArray<double> = [.3];
    let u4: FixedArray<double> = [.4];
    let u5: FixedArray<double> = [.5];
    let u6: FixedArray<double> = [.6];
    let u7: FixedArray<double> = [.7];
    let u8: FixedArray<double> = [.8];
    let u9: FixedArray<double> = [.9];
    for (let i = 0; i < iters; ++i) {
        PassArrays(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function arrGetter(iters: int): void {
    let GetArray: ()=>FixedArray<double> = STSCallee.GetArray;
    for (let i = 0; i < iters; ++i) {
        let r = GetArray();
    }
}

function strGetter(iters: int): void {
    let GetString: ()=>string = STSCallee.GetString;
    for (let i = 0; i < iters; ++i) {
        let r = GetString();
    }
}

function numGetter(iters: int): void {
    let GetNumber: ()=>double = STSCallee.GetNumber;
    for (let i = 0; i < iters; ++i) {
        let r = GetNumber();
    }
}

function objGetter(iters: int): void {
    let GetObject: ()=>Object = STSCallee.GetObject;
    for (let i = 0; i < iters; ++i) {
        let r = GetObject();
    }
}


// interop:
function interop_voidPasser(iters: int): void {
    let PassVoid = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "Pass");
    for (let i = 0; i < iters; ++i) {
        $jscall.void$(PassVoid, JSRuntime.getUndefined());
    }
}

function interop_strPasser(iters: int): void {
    let PassStrings = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "Pass");
    let u0 = "foo0";
    let u1 = "foo1";
    let u2 = "foo2";
    let u3 = "foo3";
    let u4 = "foo4";
    let u5 = "foo5";
    let u6 = "foo6";
    let u7 = "foo7";
    let u8 = "foo8";
    let u9 = "foo9";
    for (let i = 0; i < iters; ++i) {
        $jscall.void$10s(PassStrings, JSRuntime.getUndefined(), u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function interop_numPasser(iters: int): void {
    let PassNumbers = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "Pass");
    let u0 = .0;
    let u1 = .1;
    let u2 = .2;
    let u3 = .3;
    let u4 = .4;
    let u5 = .5;
    let u6 = .6;
    let u7 = .7;
    let u8 = .8;
    let u9 = .9;
    for (let i = 0; i < iters; ++i) {
        $jscall.void$10d(PassNumbers, JSRuntime.getUndefined(), u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function interop_arrPasser(iters: int): void {
    let PassArrays = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "Pass");
    let u0 : FixedArray<double> = [.0];
    let u1 : FixedArray<double> = [.1];
    let u2 : FixedArray<double> = [.2];
    let u3 : FixedArray<double> = [.3];
    let u4 : FixedArray<double> = [.4];
    let u5 : FixedArray<double> = [.5];
    let u6 : FixedArray<double> = [.6];
    let u7 : FixedArray<double> = [.7];
    let u8 : FixedArray<double> = [.8];
    let u9 : FixedArray<double> = [.9];
    for (let i = 0; i < iters; ++i) {
        $jscall.void$10a(PassArrays, JSRuntime.getUndefined(), u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
}

function interop_arrGetter(iters: int): void {
    let GetArray = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "GetArray");
    for (let i = 0; i < iters; ++i) {
        let r = $jscall.arr$(GetArray, JSRuntime.getUndefined());
    }
}

function interop_strGetter(iters: int): void {
    let GetString = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "GetString");
    for (let i = 0; i < iters; ++i) {
        let r = $jscall.str$(GetString, JSRuntime.getUndefined(),);
    }
}

function interop_numGetter(iters: int): void {
    let GetNumber = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "GetNumber");
    for (let i = 0; i < iters; ++i) {
        let r = $jscall.num$(GetNumber, JSRuntime.getUndefined());
    }
}

function interop_objGetter(iters: int): void {
    let GetObject = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "GetObject");
    for (let i = 0; i < iters; ++i) {
        let r = $jscall.any$(GetObject, JSRuntime.getUndefined());
    }
}


function interop_arrAnyGetter(iters: int): void {
    let GetArray = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "GetArray");
    for (let i = 0; i < iters; ++i) {
        let r = $jscall.any$(GetArray, JSRuntime.getUndefined());
    }
}

function interop_strAnyGetter(iters: int): void {
    let GetString = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "GetString");
    for (let i = 0; i < iters; ++i) {
        let r = $jscall.any$(GetString, JSRuntime.getUndefined(),);
    }
}

function interop_numAnyGetter(iters: int): void {
    let GetNumber = JSRuntime.getPropertyJSValue(jsvars.jsCallee, "GetNumber");
    for (let i = 0; i < iters; ++i) {
        let r = $jscall.any$(GetNumber, JSRuntime.getUndefined());
    }
}
