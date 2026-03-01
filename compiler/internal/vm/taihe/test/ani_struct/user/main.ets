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
import * as StructTest from "struct_test";

loadLibrary("ani_struct");

let structD: StructTest.StructD = StructTest.getStructD();
let structE: StructTest.StructE = StructTest.getStructE();
let structF: StructTest.StructF = StructTest.getStructF();
let structG: StructTest.StructG = StructTest.getStructG();
let structDB5: StructTest.SDB5Readonly = StructTest.getSDB5Readonly();
let structEB: StructTest.SEBReadonly = StructTest.getSEBReadonly();
function test_struct_dbase5() {
    let str: StructTest.DBase5 = {
        param1: 8,
        param2: 16,
        param3: 32,
        param4: 64,
        param5: "StructD",
    };
    structD.structD = str;

    let info = structD.structD;

    console.log("StructTest.StructD: " + str.param1);
    arktest.assertEQ(str.param1, info.param1);
}

function test_struct_ebigint() {
    let str: StructTest.EBigint = {
        param1: true,
        param2: 16.32f,
        param3: 32.16,
        param4: 641632,
    };
    structE.structE = str;

    let info = structE.structE;

    console.log("StructTest.StructE: " + str.param1);
    arktest.assertEQ(str.param1, info.param1);
}

function test_struct_funion() {
    let str: StructTest.FUnion = {
        param1: "optionalStr",
        param2: "unionStr",
        param3: {"today": "19"}
    };
    structF.structF = str;

    let info = structF.structF;

    console.log("StructTest.StructF: " + str.param1);
    arktest.assertEQ(str.param1, info.param1);
}

function test_struct_array() {
    let i8Array = new Int8Array([1, -2, 3, -4, 5]);
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrayBuffer.set(i, numbersU8[i]);
    }

    let str: StructTest.GArray = {param1: i8Array, param2: arrayBuffer};
    structG.gArray = str;
    let info = structG.gArray;

    console.log("StructTest.GArray: " + str.param1);
    console.log("StructTest.GArray: " + info.param1);
    arktest.assertEQ(str.param1[0], info.param1[0]);
}

function test_struct_addNewCanvas() {
    let canvas1 = StructTest.addNewCanvas("RedCanvas");

    console.log("canvas1's name is " + canvas1.name);
    canvas1.r += 255;
    canvas1.g += 0;
    canvas1.b += 0;
    console.log(
        "canvas1's color is " + canvas1.r + "," + canvas1.g + "," + canvas1.b);
}

function test_struct_dbase5readonly() {
    let info = structDB5.dB5Readonly;

    console.log("StructTest.DB5Readonly: " + info.param1);
    arktest.assertEQ(info.param1, (1).toByte());
}

function test_struct_sebreadonly() {
    let info = structEB.sEBReadonly;

    console.log("StructTest.SEBReadonly: " + info.param1);
    arktest.assertEQ(info.param1, true);
}

function test_struct_addNewDraw() {
    let draw = StructTest.addNewDraw("addNewDraw");

    console.log("draw's name is " + draw.name);
    draw.r += 255;
    draw.g += 255;
    draw.b += 255;
    console.log("draw's color is " + draw.r + "," + draw.g + "," + draw.b);
}

function test_struct_process_student() {
    let student: StructTest.Student = {name: "Jack", age: 10};
    let pro_student = StructTest.process_student(student);
    console.log("process student: ", pro_student.name, pro_student.age);
    arktest.assertEQ(pro_student.name, "Jack student");
    arktest.assertEQ(pro_student.age, 20);
}

function test_struct_process_teacher() {
    let teacher: StructTest.Teacher = new StructTest.Teacher("Jony", 30);
    let pro_teacher = StructTest.process_teacher(teacher);
    console.log("process teacher:", pro_teacher.name, pro_teacher.age);
    arktest.assertEQ(pro_teacher.name, "Jony teacher");
    arktest.assertEQ(pro_teacher.age, 45);
}

function test_struct_process_g() {
    let g: StructTest.G = {f: 0, g: 0};
    let new_g = StructTest.process_g(g);
    console.log("process g:", new_g.f, new_g.g);
    arktest.assertEQ(new_g.f, 1);
    arktest.assertEQ(new_g.g, 2);
}

function test_struct_process_h() {
    let h = new StructTest.H(0, 0, 0);
    let new_h = StructTest.process_h(h);
    console.log("process h:", new_h.f, new_h.g, new_h.h);
    arktest.assertEQ(new_h.f, 1);
    arktest.assertEQ(new_h.g, 2);
    arktest.assertEQ(new_h.h, 3);
}

function main() {
    let a: StructTest.A = {
        param1: 1,
    };
    let b: StructTest.B = {
        param1: 2,
        param2: 3,
    };
    let c: StructTest.C = new StructTest.C(4, 5, 6);

    console.log("############## ani_struct start #############");
    const suite = new arktest.ArkTestsuite("BundleManager Tests");

    console.log("a:", a);
    console.log("b:", b);
    console.log("c:", c);

    suite.addTest("test_struct_dbase5", test_struct_dbase5);
    suite.addTest("test_struct_ebigint", test_struct_ebigint);
    suite.addTest("test_struct_funion", test_struct_funion);
    suite.addTest("test_struct_array", test_struct_array);
    suite.addTest("test_struct_addNewCanvas", test_struct_addNewCanvas);
    suite.addTest("test_struct_dbase5readonly", test_struct_dbase5readonly);
    suite.addTest("test_struct_sebreadonly", test_struct_sebreadonly);
    suite.addTest("test_struct_addNewDraw", test_struct_addNewDraw);
    suite.addTest("test_struct_process_student", test_struct_process_student);
    suite.addTest("test_struct_process_teacher", test_struct_process_teacher);
    suite.addTest("test_struct_process_g", test_struct_process_g);
    suite.addTest("test_struct_process_h", test_struct_process_h);

    exit(suite.run());
}