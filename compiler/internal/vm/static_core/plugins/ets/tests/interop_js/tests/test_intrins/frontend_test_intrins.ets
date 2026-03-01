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

import {
    identity,
    sum,
    stringifyValue,
    makeCar,
    applyArgs,
    throwValue,
    log,
    makeTestProxy,
    stringifyArgs,
    getProp,
    nop,
    nonexisting
} from "js_code";

function vstringify(val: JSValue): String {
    return stringifyValue(val) as String;
}
function unreachable(): Error {
    log("unreachable reached!");
    return new Error("unreachable");
}
function throwmsg(msg: String): boolean {
    throw new Error(msg);
}
class BadClass {
    _init: boolean = throwmsg("error from BadClass static ctor");
};

function jsSumWrapperNumNum(a: double, b: double): double {
    return sum(a, b);
}
function jsSumWrapperNumStr(a: double, b: String): String {
    return sum(a, b) as String;
}

function testUndefined(): boolean {
    let v = JSRuntime.getUndefined();
    let r = "undefined:undefined";
    if (identity(v) as Any !== undefined) {
        return false;
    }
    return r == vstringify(v);
}
function testNull(): boolean {
    let v = JSRuntime.getNull();
    let r = "object:null";
    return r == vstringify(identity(v));
}
function testBoolean(): boolean {
    let v = true;
    let r = "boolean:true";
    v = identity(identity(v)) as boolean;
    return r == vstringify(identity(v));
}
function testNumber(): boolean {
    let v: double = 42.24;
    let r = "number:42.24";
    v = identity(identity(v)) as double;
    return r == vstringify(identity(v));
}
function testString(): boolean {
    let v = "foo12345678901234567890"
    let r = "string:foo12345678901234567890";
    v = identity(identity(v)) as String;
    return r == vstringify(identity(v));
}
function testObject(): boolean {
    let v = JSRuntime.createObject();
    let r = "object:[object Object]"
    return r == vstringify(identity(v));
}
function testBigint(): boolean {
    let v: bigint = 1234567890123456789012345678901234567890n
    let r = "bigint:1234567890123456789012345678901234567890";
    v = identity(identity(v)) as BigInt;
    return r == vstringify(identity(v));
}

function testStringOps(): boolean {
    let strv = "foo";
    let v1: JSValue = strv;
    if (v1 as String != strv) {
        return false;
    }
    if (getProp(strv, "length") as double != strv.length) {
        return false;
    }
    if (identity(identity(strv)) as String != strv) {
        return false;
    }
    return true;
}

function testBuiltinArrayAny(): boolean {
    let v: FixedArray<JSValue> = [JSRuntime.getUndefined(), JSRuntime.newJSValueDouble(1.23), JSRuntime.newJSValueString("foo"), JSRuntime.createObject()];
    let r = "object:,1.23,foo,[object Object]";
    // v = identity(identity(v)) as FixedArray<JSValue>; // TODO(vpukhov): jscast
    let v1 = identity(identity(v));
    return r == vstringify(identity(v1));
}
function testBuiltinArrayBoolean(): boolean {
    let v: FixedArray<boolean> = [true, false, true];
    let r = "object:true,false,true";
    let v1 = identity(identity(v));
    return r == vstringify(identity(v1));
}
function testBuiltinArrayInt(): boolean {
    let v: FixedArray<int> = [111, 222];
    let r = "object:111,222";
    let v1 = identity(identity(v));
    return r == vstringify(identity(v1));
}
function testBuiltinArrayNumber(): boolean {
    let v: FixedArray<double> = [11.1, 22.2];
    let r = "object:11.1,22.2";
    let v1 = identity(identity(v));
    return r == vstringify(identity(v1));
}
function testBuiltinArrayString(): boolean {
    let v: FixedArray<String> = ["foo0", "foo1"];
    let r = "object:foo0,foo1";
    let v1 = identity(identity(v));
    return r == vstringify(identity(v1));
}
function testBuiltinArrayObject(): boolean {
    let v: FixedArray<Object> = [JSRuntime.getUndefined(), JSRuntime.createObject(), "foo", 1];
    let r = "object:,[object Object],foo,1";
    let v1 = identity(identity(v));
    return r == vstringify(identity(v1));
}
function testBuiltinArrayMultidim(): boolean {
    let v: FixedArray<FixedArray<double>> = [[11.1, 22.2], [33.3, 44.4]];
    let r = "object:11.1,22.2,33.3,44.4";
    let v1 = identity(identity(v));
    return r == vstringify(identity(v1));
}
function testBuiltinArrayInstanceof(): boolean {
    let v: FixedArray<int> = [111, 222];
    let ctor = JSRuntime.getGlobal().Array;
    return JSRuntime.instanceOfDynamic(identity(v), ctor);
}
function testInitArrayComponent(): boolean {
    // TODO(vpukhov): relies on jscast FixedArray<BadClass>
    return true;
}

function testNamedAccess(): boolean {
    let v: double = 12.34
    let obj = JSRuntime.createObject();
    obj.prop = v;
    return v == obj.prop as double;
}

function testNewcall(): boolean {
    let color = "Cyan";
    let makeCar = new makeCar(color);
    return color == makeCar.color as String;
}

function testValueCall(): boolean {
    return 123.4 == identity(123.4) as double;
}

function testCallStress(): boolean {
    let workload = 128;
    let u = JSRuntime.getUndefined();
    for (let i = 0; i < workload; ++i) {
        nop(u, u, u, u)
    }
    return true;
}

// Remove after JSValue cast fix
function testUndefinedCastBug(): boolean {
    let v = JSRuntime.getUndefined();
    let r = "undefined:undefined";
    return r == vstringify(identity(v));
}

function testLambdaProxy(): boolean {
    let etsfn: (x: double) => double = (x: double): double => {
        return x;
    }

    let res = applyArgs(etsfn, 123) as int;
    return res == 123;
}

function testLambdaProxyRecursive(): boolean {
    let etsfn: (x: double, y: String) => String = (x: double, y: String): String => {
        return stringifyArgs(x, y) as String;
    }

    let res = applyArgs(etsfn, 123, "foo") as String;
    return res == "123,foo";
}

function testExceptionForwardingFromjs(): boolean {
    try {
        throwValue(123.321);
    } catch (e) {
        if (e instanceof ESError) {
            return "number:123.321" == vstringify(e.getJsError());
        }
    }
    throw unreachable();
}

function testExceptionForwardingRecursive(): boolean {
    let etsfn: (v: JSValue) => void = (v: JSValue): void => {
        throwValue(v);
        throw unreachable();
    }

    try {
        applyArgs(etsfn, 123.456);
    } catch (e) {
        if (e instanceof ESError) {
            return "number:123.456" == vstringify(e.getJsError());
        }
    }
    throw unreachable();
}

function testCoreErrorForwarding(): boolean {
    let etsfn: () => void = (): void => {
        throw new Error("funny message");
    }

    try {
        applyArgs(etsfn);
    } catch (e) {
        return e.message == "funny message";
    }
    throw unreachable();
}

function testTypecheckGetProp(): boolean {
    let obj = JSRuntime.createObject();
    obj.prop = 1.23;
    try {
        obj.prop as String;
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: String expected" == vstringify(e);
        }
    }
    throw unreachable();
}

function testTypecheckJscall(): boolean {
    try {
        identity(123) as String;
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: String expected" == vstringify(e);
        }
    }
    throw unreachable();
}

function testTypecheckCallets(): boolean {
    try {
        let etsfn: (x: String) => String = (x: String): String => {
            throw unreachable();
        }

        applyArgs(etsfn, 123);
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: String expected" == vstringify(e);
        }
    }
    throw unreachable();
}

function testGetThrows(): boolean {
    let obj = new makeTestProxy();
    try {
        obj.foo as double;
    } catch (e) {
        if (e instanceof ESError) {
            return "object:Error: get exception" == vstringify(e.getJsError());
        }
    }
    throw unreachable();
}

function testSetThrows(): boolean {
    let obj = new makeTestProxy();
    try {
        obj.foo = 123;
    } catch (e) {
        if (e instanceof ESError) {
            return "object:Error: set exception" == vstringify(e.getJsError());
        }
    }
    throw unreachable();
}

function testJscallResolutionThrows1(): boolean {
    try {
        nonexisting();
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: call target is not a function" == vstringify(e);
        }
    }
    throw unreachable();
}

function testJscallResolutionThrows2(): boolean {
    try {
        nonexisting.nonexisting();
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: Cannot convert undefined or null to object" == vstringify(e);
        }
    }
    throw unreachable();
}

function testJscallResolutionThrows3(): boolean {
    let obj = new makeTestProxy();
    try {
        obj.foo();
    } catch (e) {
        if (e instanceof ESError) {
            return "object:Error: get exception" == vstringify(e.getJsError());
        }
    }
    throw unreachable();
}

function gcJsruntimeCleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let fullGcId = GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE);
        // We need to switch from main coroutine to coroutine created by FinalizationRegistry because
        // if main coro is executed first, the test will return false
        Coroutine.Schedule();
        GC.waitForFinishGC(fullGcId);
    } catch (e) {
        arktest.assertTrue( false, "Unexpected exception during GC");
    }
}

function testFinalizers(): boolean {
    let workload = 128;
    let repeats = 4;

    for (let k = 0; k < repeats; ++k) {
        gcJsruntimeCleanup();
        let numRefsPre = JSRuntime.getNumSharedJSValues();

        for (let i = 0; i < workload; ++i) {
            JSRuntime.createObject();
        }
        for (let i = 0; i < workload; ++i) {
            JSRuntime.newJSValueString("foo");
        }
        for (let i = 0; i < workload; ++i) {
            JSRuntime.newJSValueDouble(1234);
        }

        arktest.assertEQ( JSRuntime.getNumSharedJSValues(), numRefsPre + workload);
        gcJsruntimeCleanup();
        arktest.assertEQ( JSRuntime.getNumSharedJSValues(), numRefsPre);
    }
    return true;
}
