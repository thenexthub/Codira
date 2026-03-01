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
    ADeclared,
    BDeclared,
    CDeclared,
    DDeclared,
    asJsvalue,
    createError,
} from "dynamic_import_with_decl"

let module = ESValue.load('js')
let AValue = module.getProperty('AValue');
let CValue = module.getProperty('CValue');
let jsAvalue = module.getProperty('jsAvalue');
let jsCvalue = module.getProperty('jsCvalue');
let jsFn = module.getProperty('jsFn');

class A { }
class C extends A { }

function errLog(op: String) {
    console.println("Bug operation: " + op);
}

/*
 * =====================================
 * Tests <object type>InstanceofObject
 * =====================================
 */

function TestObjectInstanceofObject(): boolean {
    let o = new Object();
    if (!(o instanceof Object)) {
        errLog("o instanceof Object");
        return false;
    }
    return true;
}

function TestEtstypeInstanceofObject(): boolean {
    let a = new A();
    if (!(a instanceof Object)) {
        errLog("a instanceof Object");
        return false;
    }
    return true;
}

function TestJsvalueInstanceofObject(): boolean {
    // Check js object as left argument
    if (!(jsAvalue.unwrap() instanceof Object)) {
        errLog("js_avalue instanceof Object");
        return false;
    }
    let jsAvalueX = asJsvalue(jsAvalue.unwrap());
    if (!(jsAvalueX instanceof Object)) {
        errLog("js_avalue_x instanceof Object");
        return false;
    }

    // Check ets object as left argumet
    let a = new A();
    let aJsvalue: Any = asJsvalue(a);
    if (!(aJsvalue instanceof Object)) {
        errLog("a_jsvalue instanceof Object");
        return false;
    }
    return true;
}

function TestDynvalueInstanceofObject(): boolean {
    let avalue: ESValue = AValue.instantiate();
    if (!(avalue.unwrap() instanceof Object)) {
        errLog("avalue instanceof Object");
        return false;
    }
    let cvalue: ESValue = CValue.instantiate();
    if (!(cvalue.unwrap() instanceof Object)) {
        errLog("cvalue instanceof Object");
        return false;
    }
    return true;
}

function TestDyndeclInstanceofObject(): boolean {
    let adecl = new ADeclared();
    if (!(adecl instanceof Object)) {
        errLog("adecl instanceof Object");
        return false;
    }
    return true;
}

/*
 * ==============================================
 * === Tests <object type>InstanceofEtstype ===
 * ==============================================
 */

function TestObjectInstanceofEtstype(): boolean {
    let o = new Object();
    if (o instanceof A) {
        errLog("o instanceof A");
        return false;
    }
    return true;
}

function TestEtstypeInstanceofEtstype(): boolean {
    let a = new A();
    if (!(a instanceof A)) {
        errLog("a instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //let b = new B();
    //if (!(b instanceof A)) {
    //    errLog("b instanceof A");
    //    return false;
    //}
    let c = new C();
    if (!(c instanceof A)) {
        errLog("c instanceof A");
        return false;
    }
    return true;
}

function TestJsvalueInstanceofEtstype(): boolean {
    // Check js object as left argument
    if (jsAvalue.unwrap() instanceof A) {
        errLog("js_avalue instanceof A");
        return false;
    }
    let jsAvalueX = asJsvalue(jsAvalue.unwrap());
    if (jsAvalueX instanceof A) {
        errLog("js_avalue_x instanceof A");
        return false;
    }

    // Check ets object as left argumet
    let a = new A();
    let aJsvalue: Any = asJsvalue(a);
    if (!(aJsvalue instanceof A)) {
        errLog("a_jsvalue instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //if (aJsvalue instanceof B) {
    //    errLog("a_jsvalue instanceof B");
    //    return false;
    //}
    let c = new C();
    let cJsvalue: Any = asJsvalue(c);
    if (!(cJsvalue instanceof A)) {
        errLog("c_jsvalue instanceof A");
        return false;
    }
    return true;
}

function TestDynvalueInstanceofEtstype(): boolean {
    let avalue: ESValue = AValue.instantiate();
    if (avalue.unwrap() instanceof A) {
        errLog("avalue instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //let cvalue: ESValue = CValue.instantiate();
    //if (!(cvalue instanceof B)) {
    //    errLog("cvalue instanceof B");
    //    return false;
    //}
    return true;
}

function TestDyndeclInstanceofEtstype(): boolean {
    let adecl = new ADeclared();
    if (adecl instanceof A) {
        errLog("adecl instanceof A");
        return false;
    }
    let ddecl = new DDeclared();
    if (!(ddecl instanceof A)) {
        errLog("ddecl instanceof A");
        return false;
    }

    // Check compat class
    let etsError = new Error("ets error message")
    let etsErrorJsval: Any = etsError;
    if (!(etsErrorJsval instanceof Error)) {
        errLog("etsErrorJsval instanceof Error");
        return false;
    }

    let jsError: Error = createError("error message");
    if (!(jsError instanceof Error)) {
        errLog("jsError instanceof Error");
        return false;
    }
    // TODO(v.cherkashin): Enable when processing the compat types will be implemented, #13577
    //let jsErrorJsval: Any = jsError;
    //if (!(js_error_jsval instanceof Error)) {
    //    errLog("js_error_jsval instanceof Error");
    //    return false;
    //}
    return true;
}

/*
 * ==============================================
 * === Tests <object type>InstanceofDyndecl ===
 * ==============================================
 */

function TestObjectInstanceofDyndecl(): boolean {
    // Check 'object instanceof dyndecl'
    let o = new Object();
    if (o instanceof BDeclared) {
        errLog("o instanceof BDeclared");
        return false;
    }
    return true;
}

function TestEtstypeInstanceofDyndecl(): boolean {
    // Check 'etstype instanceof dyndecl'
    let a = new A();
    if (a instanceof BDeclared) {
        errLog("a instanceof BDeclared");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance with declared type will be implemented, #13552
    //let b = new B();
    //if (!(b instanceof BDeclared)) {
    //    errLog("b instanceof BDeclared");
    //    return false;
    //}
    return true;
}

function TestJsvalueInstanceofDyndecl(): boolean {
    // Check 'jsvalue instanceof dyndecl'
    if (jsAvalue.unwrap() instanceof BDeclared) {
        errLog("js_avalue instanceof BDeclared");
        return false;
    }
    if (!(jsCvalue.unwrap() instanceof BDeclared)) {
        errLog("jsCvalue instanceof BDeclared");
        return false;
    }
    return true;
}

function TestDynvalueInstanceofDyndecl(): boolean {
    // Check 'dynvalue instanceof dyndecl'
    let avalue: ESValue = AValue.instantiate();
    if (avalue.unwrap() instanceof BDeclared) {
        errLog("avalue instanceof BDeclared");
        return false;
    }
    let cvalue: ESValue = CValue.instantiate();
    if (!(cvalue.unwrap() instanceof BDeclared)) {
        errLog("cvalue instanceof BDeclared");
        return false;
    }
    return true;
}

function TestDyndeclInstanceofDyndecl(): boolean {
    let adecl: ADeclared = new ADeclared();
    if (adecl instanceof BDeclared) {
        errLog("adecl instanceof BDeclared");
        return false;
    }
    let bdecl: BDeclared = new BDeclared();
    if (!(bdecl instanceof BDeclared)) {
        errLog("bdecl instanceof BDeclared");
        return false;
    }
    let cdecl: CDeclared = new CDeclared();
    if (cdecl instanceof BDeclared) {
        errLog("cdecl instanceof BDeclared");
        return false;
    }
    if (!(cdecl instanceof ADeclared)) {
        errLog("cdecl instanceof ADeclared");
        return false;
    }

    // Check 'js_fn instanceof dyndecl'
    if (jsFn.unwrap() instanceof BDeclared) {
        errLog("js_fn instanceof BDeclared");
        return false;
    }

    return true;
}
