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
import {BusinessError} from "@ohos.base";
import * as on_off_test from "on_off";

loadLibrary("ani_on_off");

function ibase_on() {
    let a = on_off_test.getIBase("hello", "world")
    try {
        a.on("set", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function ibase_off() {
    let a = on_off_test.getIBase("hello", "world")
    try {
        a.off("set", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevel_foo_on() {
    // toplevel func on
    try {
        on_off_test.on("foo", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevel_bar_on() {
    // toplevel func on
    try {
        on_off_test.on("bar", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevel_my_baz_on() {
    // toplevel func on
    try {
        on_off_test.on("my_baz", 1, () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevel_foo_off() {
    // toplevel func on
    try {
        on_off_test.off("foo", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevel_bar_off() {
    // toplevel func on
    try {
        on_off_test.off("bar", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevel_my_baz_off() {
    // toplevel func on
    try {
        on_off_test.off("my_baz", 2, () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function undefined_on() {
    try {
        on_off_test.on("ABC", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function undefined_off() {
    try {
        on_off_test.off("ABC", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function basecls_static_on() {
    try {
        on_off_test.BaseCls.on("fooStatic", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function basecls_static_off() {
    try {
        on_off_test.BaseCls.off("fooStatic", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevelFuncIOn() {
    // toplevel func on
    try {
        on_off_test.on("funcI", (arg0: int): void => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevelFuncBOn() {
    // toplevel func on
    try {
        on_off_test.on("funcB", (arg0: boolean): void => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevelFuncIOff() {
    // toplevel func off
    try {
        on_off_test.off("funcI", (arg0: int): void => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function toplevelFuncBOff() {
    // toplevel func off
    try {
        on_off_test.off("funcB", (arg0: boolean): void => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function main() {
    const suite = new arktest.ArkTestsuite("on off testcase")

    suite.addTest("ibase_on", ibase_on)
    suite.addTest("ibase_off", ibase_off)
    suite.addTest("toplevel_foo_on", toplevel_foo_on)
    suite.addTest("toplevel_bar_on", toplevel_bar_on)
    suite.addTest("toplevel_my_baz_on", toplevel_my_baz_on)
    suite.addTest("toplevel_foo_off", toplevel_foo_off)
    suite.addTest("toplevel_bar_off", toplevel_bar_off)
    suite.addTest("toplevel_my_baz_off", toplevel_my_baz_off)
    suite.addTest("undefined_on", undefined_on)
    suite.addTest("undefined_off", undefined_off)
    suite.addTest("basecls_static_on", basecls_static_on)
    suite.addTest("basecls_static_off", basecls_static_off)
    suite.addTest("toplevelFuncIOn", toplevelFuncIOn)
    suite.addTest("toplevelFuncBOn", toplevelFuncBOn)
    suite.addTest("toplevelFuncIOff", toplevelFuncIOff)
    suite.addTest("toplevelFuncBOff", toplevelFuncBOff)
    exit(suite.run())
}
