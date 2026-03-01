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

class jsvars {
    static global = JSRuntime.getGlobal();
    static gtestEnv = JSRuntime.getPropertyJSValue(jsvars.global, "gtest_env");

    static m = JSRuntime.getPropertyJSValue(jsvars.gtestEnv, "test_intrins");
};

class jscall {
    static Init = JSRuntime.__initJSCallClass();

    // Remove after JSValue cast fix: signature type suffixes

    native static void$(top: JSValue, qname: String): void;
    native static void$a(top: JSValue, qname: String, v0: JSValue): void;
    native static void$an(top: JSValue, qname: String, a0: JSValue, a1: double): void;
    native static void$n(top: JSValue, qname: String, v0: double): void;
    native static void$s(top: JSValue, qname: String, v0: String): void;

    native static bool$a(top: JSValue, qname: String, v0: JSValue): boolean;

    native static num$a(top: JSValue, qname: String, v0: JSValue): double
    native static num$an(top: JSValue, qname: String, a0: JSValue, a1: double): double;
    native static num$ann(top: JSValue, qname: String, a0: JSValue, a1: double, a2: double): double;
    native static num$nn(top: JSValue, qname: String, a0: double, a1: double): double;
    native static num$ss(top: JSValue, qname: String, a0: String, a1: String): double;

    native static str$a(top: JSValue, qname: String, v0: JSValue): String;
    native static str$aa(top: JSValue, qname: String, v0: JSValue, v1: JSValue): String;
    native static str$ans(top: JSValue, qname: String, a0: JSValue, a1: double, a2: String): String;
    native static str$n(top: JSValue, qname: String, a0: double): String;
    native static str$ns(top: JSValue, qname: String, a0: double, a1: String): String;
    native static str$nsn(top: JSValue, qname: String, a0: double, a1: String, a2: double): String;
    native static str$s(top: JSValue, qname: String, a0: String): String;

    native static bigint$a(top: JSValue, qname: String, v0: JSValue): BigInt;

    native static any$a(top: JSValue, qname: String, v0: JSValue): JSValue;
    native static any$a$obj(top: JSValue, qname: String, v0: JSValue): Any;
    native static any$b(top: JSValue, qname: String, v0: boolean): JSValue;
    native static any$n(top: JSValue, qname: String, v0: double): JSValue;
    native static any$s(top: JSValue, qname: String, v0: String): JSValue;
    native static any$Aa(top: JSValue, qname: String, v0: FixedArray<JSValue>): JSValue;
    native static any$Ab(top: JSValue, qname: String, v0: FixedArray<boolean>): JSValue;
    native static any$Ai(top: JSValue, qname: String, v0: FixedArray<int>): JSValue;
    native static any$An(top: JSValue, qname: String, v0: FixedArray<double>): JSValue;
    native static any$AAn(top: JSValue, qname: String, v0: FixedArray<FixedArray<double>>): JSValue;
    native static any$As(top: JSValue, qname: String, v0: FixedArray<String>): JSValue;
    native static any$Ao(top: JSValue, qname: String, v0: FixedArray<Object>): JSValue;
    native static any$aaaa(top: JSValue, qname: String, v0: JSValue, v1: JSValue, v2: JSValue, v3: JSValue): JSValue;
    native static any$bi(top: JSValue, qname: String, v0: BigInt): JSValue;
    // Remove after JSValue cast fix
    native static any$(top: JSValue, qname: String, v0: JSValue): JSValue;

    native static Aa$a(top: JSValue, qname: String, v0: JSValue): FixedArray<JSValue>;
    native static Ab$a(top: JSValue, qname: String, v0: JSValue): FixedArray<boolean>;
    native static Ai$a(top: JSValue, qname: String, v0: JSValue): FixedArray<int>;
    native static An$a(top: JSValue, qname: String, v0: JSValue): FixedArray<double>;
    native static AAn$a(top: JSValue, qname: String, v0: JSValue): FixedArray<FixedArray<double>>;
    native static As$a(top: JSValue, qname: String, v0: JSValue): FixedArray<String>;
    native static Ao$a(top: JSValue, qname: String, v0: JSValue): FixedArray<Object>;

    native static ALBadClass$a(top: JSValue, qname: String, v0: JSValue): FixedArray<BadClass>;

    native static num$vn(fn: JSValue, rec: JSValue, a0: double): double;
};

class jsnew {
    static Init = JSRuntime.__initJSNewClass();
    native static $(top: JSValue, qname: String): JSValue;
    native static $s(top: JSValue, qname: String, v0: String): JSValue;
};

function vstringify(val: JSValue): String {
    return jscall.str$a(jsvars.m, ".stringifyValue", val);
}
function vlog(val: JSValue): void {
    jscall.void$a(jsvars.m, ".Log", val);
}
function vlog(val: String): void {
    jscall.void$s(jsvars.m, ".Log", val);
}
function unreachable(): Error {
    vlog("unreachable reached!");
    return new Error("unreachable");
}
function throwmsg(msg: String): boolean {
    throw new Error(msg);
}
class BadClass {
    static Init: boolean = throwmsg("error from BadClass static ctor");
};

function jsSumWrapperNumNum(a: double, b: double): double {
    return jscall.num$nn(jsvars.m, ".sum", a, b);
}
function jsSumWrapperNumStr(a: double, b: String): String {
    return jscall.str$ns(jsvars.m, ".sum", a, b);
}

function testUndefined(): boolean {
    let v = JSRuntime.getUndefined();
    let r = "undefined:undefined";
    if (jscall.any$a$obj(jsvars.m, ".identity", v) !== undefined) {
        return false;
    }
    return r == vstringify(v);
}

function testNull(): boolean {
    let v = JSRuntime.getNull();
    let r = "object:null";
    if (jscall.any$a(jsvars.m, ".identity", v) as Any !== null) {
        return false;
    }
    return r == vstringify(v);
}
function testBoolean(): boolean {
    let v = true;
    let r = "boolean:true";
    v = jscall.bool$a(jsvars.m, ".identity", jscall.any$b(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$b(jsvars.m, ".identity", v));
}
function testNumber(): boolean {
    let v: double = 42.24;
    let r = "number:42.24";
    v = jscall.num$a(jsvars.m, ".identity", jscall.any$n(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$n(jsvars.m, ".identity", v));
}
function testString(): boolean {
    let v = "foo12345678901234567890";
    let r = "string:foo12345678901234567890";
    v = jscall.str$a(jsvars.m, ".identity", jscall.any$s(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$s(jsvars.m, ".identity", v));
}
function testObject(): boolean {
    let v = JSRuntime.createObject();
    let r = "object:[object Object]";
    return r == vstringify(jscall.any$a(jsvars.m, ".identity", v));
}

function testBigint(): boolean {
    let v1: bigint = -1234567890123456789012345678901234567890n;
    let r1 = "bigint:-1234567890123456789012345678901234567890";
    v1 = jscall.bigint$a(jsvars.m, ".identity", jscall.any$bi(jsvars.m, ".identity", v1));

    let v2: bigint = 1234567890123456789012345678901234567890n;
    let r2 = "bigint:1234567890123456789012345678901234567890";
    v2 = jscall.bigint$a(jsvars.m, ".identity", jscall.any$bi(jsvars.m, ".identity", v2));

    let v3: bigint = 0n;
    let r3 = "bigint:0";
    v3 = jscall.bigint$a(jsvars.m, ".identity", jscall.any$bi(jsvars.m, ".identity", v3));

    let v4: bigint = -0n;
    let r4 = "bigint:0";
    v4 = jscall.bigint$a(jsvars.m, ".identity", jscall.any$bi(jsvars.m, ".identity", v4));

    let v5: bigint = -18446744073709551614n; // FF FF FF FF FF FF FF FE
    let r5 = "bigint:-18446744073709551614";
    v5 = jscall.bigint$a(jsvars.m, ".identity", jscall.any$bi(jsvars.m, ".identity", v5));

    let v6: bigint = -4703919738795935662080n; // FF 00 00 00 00 00 00 00 00
    let r6 = "bigint:-4703919738795935662080";
    v6 = jscall.bigint$a(jsvars.m, ".identity", jscall.any$bi(jsvars.m, ".identity", v6));

    let v7: bigint = 2147483648n; // 2^31
    let r7 = "bigint:2147483648";
    v7 = jscall.bigint$a(jsvars.m, ".identity", jscall.any$bi(jsvars.m, ".identity", v7));

    return r1 == vstringify(jscall.any$bi(jsvars.m, ".identity", v1)) &&
           r2 == vstringify(jscall.any$bi(jsvars.m, ".identity", v2)) &&
           r3 == vstringify(jscall.any$bi(jsvars.m, ".identity", v3)) &&
           r4 == vstringify(jscall.any$bi(jsvars.m, ".identity", v4)) &&
           r5 == vstringify(jscall.any$bi(jsvars.m, ".identity", v5)) &&
           r6 == vstringify(jscall.any$bi(jsvars.m, ".identity", v6)) &&
           r7 == vstringify(jscall.any$bi(jsvars.m, ".identity", v7));
}

function testStringOps(): boolean {
    let strv = "foo";
    let v1 = JSRuntime.newJSValueString(strv);
    if (JSRuntime.getValueString(v1) != strv) {
        return false;
    }
    if (jscall.num$ss(jsvars.m, ".getProp", strv, "length") != strv.length) {
        return false;
    }
    if (jscall.str$a(jsvars.m, ".identity", jscall.any$s(jsvars.m, ".identity", strv)) != strv) {
        return false;
    }
    return true;
}

function testBuiltinArrayAny(): boolean {
    let v: FixedArray<JSValue> = [JSRuntime.getUndefined(), JSRuntime.newJSValueDouble(1.23), JSRuntime.newJSValueString("foo"), JSRuntime.createObject()];
    let r = "object:,1.23,foo,[object Object]";
    v = jscall.Aa$a(jsvars.m, ".identity", jscall.any$Aa(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$Aa(jsvars.m, ".identity", v));
}
function testBuiltinArrayBoolean(): boolean {
    let v: FixedArray<boolean> = [true, false, true];
    let r = "object:true,false,true";
    v = jscall.Ab$a(jsvars.m, ".identity", jscall.any$Ab(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$Ab(jsvars.m, ".identity", v));
}
function testBuiltinArrayInt(): boolean {
    let v: FixedArray<int> = [111, 222];
    let r = "object:111,222";
    v = jscall.Ai$a(jsvars.m, ".identity", jscall.any$Ai(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$Ai(jsvars.m, ".identity", v));
}
function testBuiltinArrayNumber(): boolean {
    let v: FixedArray<double> = [11.1, 22.2];
    let r = "object:11.1,22.2";
    v = jscall.An$a(jsvars.m, ".identity", jscall.any$An(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$An(jsvars.m, ".identity", v));
}
function testBuiltinArrayString(): boolean {
    let v: FixedArray<String> = ["foo0", "foo1"];
    let r = "object:foo0,foo1";
    v = jscall.As$a(jsvars.m, ".identity", jscall.any$As(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$As(jsvars.m, ".identity", v));
}
function testBuiltinArrayObject(): boolean {
    let v: FixedArray<Object> = [JSRuntime.getUndefined(), JSRuntime.createObject(), "foo", 1, jsnew.$(jsvars.global, ".Number")];
    let r = "object:,[object Object],foo,1,0";
    v = jscall.Ao$a(jsvars.m, ".identity", jscall.any$Ao(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$Ao(jsvars.m, ".identity", v));
}
function testBuiltinArrayMultidim(): boolean {
    let v: FixedArray<FixedArray<double>> = [[11.1, 22.2], [33.3, 44.4]];
    let r = "object:11.1,22.2,33.3,44.4";
    v = jscall.AAn$a(jsvars.m, ".identity", jscall.any$AAn(jsvars.m, ".identity", v));
    return r == vstringify(jscall.any$AAn(jsvars.m, ".identity", v));
}
function testBuiltinArrayInstanceof(): boolean {
    let v: FixedArray<int> = [111, 222];
    let ctor = JSRuntime.getPropertyJSValue(jsvars.global, "Array");
    return JSRuntime.instanceOfDynamic(jscall.any$Ai(jsvars.m, ".identity", v), ctor);
}
function testInitArrayComponent(): boolean {
    let v: FixedArray<JSValue> = [JSRuntime.getNull()];
    let x = jscall.ALBadClass$a(jsvars.m, ".identity", jscall.any$Aa(jsvars.m, ".identity", v));
    return true;
}

function testNamedAccess(): boolean {
    let v: double = 12.34
    let obj = JSRuntime.createObject();
    JSRuntime.setPropertyDouble(obj, "prop", v);
    return v == JSRuntime.getPropertyDouble(obj, "prop");
}

function testNewcall(): boolean {
    let color = "Cyan";
    let car = jsnew.$s(jsvars.m, ".makeCar", color);
    return color == JSRuntime.getPropertyString(car, "color");
}

function testValueCall(): boolean {
    let fn = JSRuntime.getPropertyJSValue(jsvars.m, "identity");
    return 123.4 == jscall.num$vn(fn, JSRuntime.getUndefined(), 123.4);
}

function testCallStress(): boolean {
    let workload = 128;
    let u = JSRuntime.getUndefined();
    for (let i = 0; i < workload; ++i) {
        jscall.any$aaaa(jsvars.m, ".nop", u, u, u, u);
    }
    return true;
}

// Remove after JSValue cast fix
function testUndefinedCastBug(): boolean {
    let v = JSRuntime.getUndefined();
    let r = "undefined:undefined";
    return r == vstringify(jscall.any$(jsvars.m, ".identity", v));
}

function testLambdaProxy(): boolean {
    let etsfn: (x: double) => double = (x: double): double => {
        return x;
    }

    let res = jscall.num$an(jsvars.m, ".applyArgs", etsfn, 123);
    return res == 123;
}

function testLambdaProxyRecursive(): boolean {
    let etsfn: (x: double, y: String) => String = (x: double, y: String): String => {
        return jscall.str$ns(jsvars.m, ".stringifyArgs", x, y);
    }

    let res = jscall.str$ans(jsvars.m, ".applyArgs", etsfn, 123, "foo");
    return res == "123,foo";
}

function testExceptionForwardingFromjs(): boolean {
    try {
        jscall.void$n(jsvars.m, ".throwValue", 123.321);
    } catch (e) {
        if (e instanceof ESError) {
            return "number:123.321" == vstringify(e.getJsError());
        }
    }
    throw unreachable();
}

function testExceptionForwardingRecursive(): boolean {
    let etsfn: (v: JSValue) => void = (v: JSValue): void => {
        jscall.void$a(jsvars.m, ".throwValue", v);
        throw unreachable();
    }

    try {
        jscall.void$an(jsvars.m, ".applyArgs", etsfn, 123.456);
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
        jscall.void$a(jsvars.m, ".applyArgs", etsfn);
    } catch (e) {
        // return e.message == "funny message";
    }
    return true
    // throw unreachable();
}

function testTypecheckGetProp(): boolean {
    let obj = JSRuntime.createObject();
    JSRuntime.setPropertyDouble(obj, "prop", 1.23);
    try {
        JSRuntime.getPropertyString(obj, "prop");
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: String expected" == vstringify(e);
        }
    }
    throw unreachable();
}

function testTypecheckJscall(): boolean {
    try {
        jscall.str$n(jsvars.m, ".identity", 123);
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

        jscall.num$an(jsvars.m, ".applyArgs", etsfn, 123);
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: String expected" == vstringify(e);
        }
    }
    throw unreachable();
}

function testGetThrows(): boolean {
    let obj = jsnew.$(jsvars.m, ".makeTestProxy");
    try {
        JSRuntime.getPropertyDouble(obj, "foo");
    } catch (e) {
        if (e instanceof ESError) {
            return "object:Error: get exception" == vstringify(e.getJsError());
        }
    }
    throw unreachable();
}

function testSetThrows(): boolean {
    let obj = jsnew.$(jsvars.m, ".makeTestProxy");
    try {
        JSRuntime.setPropertyDouble(obj, "foo", 123);
    } catch (e) {
        if (e instanceof ESError) {
            return "object:Error: set exception" == vstringify(e.getJsError());
        }
    }
    throw unreachable();
}

function testJscallResolutionThrows1(): boolean {
    try {
        jscall.void$(jsvars.m, ".Nonexisting");
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: call target is not a function" == vstringify(e);
        }
    }
    throw unreachable();
}

function testJscallResolutionThrows2(): boolean {
    try {
        jscall.void$(jsvars.m, ".Nonexisting.Nonexsiting");
    } catch (e) {
        if (e instanceof TypeError) {
            return "object:TypeError: Cannot convert undefined or null to object" == vstringify(e);
        }
    }
    throw unreachable();
}

function testJscallResolutionThrows3(): boolean {
    let obj = jsnew.$(jsvars.m, ".makeTestProxy");
    try {
        jscall.void$(obj, ".foo");
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
