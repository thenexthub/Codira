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
import * as StructExtendTest from "struct_extend";

loadLibrary("ani_struct_extend");

function check_struct_A() {
    let ins_a: StructExtendTest.A = {param1: 100};
    StructExtendTest.check_A(ins_a)
    let res_a = StructExtendTest.create_A()
    arktest.assertEQ(res_a.param1, 1)
}

function check_struct_B() {
    let ins_b: StructExtendTest.B = {param1: 100, param2: 200};
    StructExtendTest.check_B(ins_b)
    let res_b = StructExtendTest.create_B()
    arktest.assertEQ(res_b.param1, 1)
    arktest.assertEQ(res_b.param2, 2)
}

class C implements StructExtendTest.C {
    param1: int = 100;
    param2: int = 200;
    param3: int = 300;
}

function check_struct_C() {
    let ins_c: StructExtendTest.C = new C();
    StructExtendTest.check_C(ins_c)
    let res_c = StructExtendTest.create_C()
    arktest.assertEQ(res_c.param1, 1)
    arktest.assertEQ(res_c.param2, 2)
    arktest.assertEQ(res_c.param3, 3)
}

function check_struct_D() {
    let ins_d: StructExtendTest.D = {param4: 400};
    StructExtendTest.check_D(ins_d)
    let res_d = StructExtendTest.create_D()
    arktest.assertEQ(res_d.param4, 4)
}

function check_struct_E() {
    let ins_e: StructExtendTest.E = {param4: 400, param5: 500};
    StructExtendTest.check_E(ins_e)
    let res_e = StructExtendTest.create_E()
    arktest.assertEQ(res_e.param4, 4)
    arktest.assertEQ(res_e.param5, 5)
}

function check_class_Bar() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let bar: StructExtendTest.Bar = StructExtendTest.create_Bar(in_e);
    arktest.assertEQ(StructExtendTest.check_Bar(bar), true);
    let e: StructExtendTest.E = bar.getE();
    arktest.assertEQ(e.param4, 4);
    arktest.assertEQ(e.param5, 5);
    let e1: StructExtendTest.E = {param4: 400, param5: 500};
    bar.setE(e1);
    let e2: StructExtendTest.E = bar.getE();
    arktest.assertEQ(e2.param4, 400);
    arktest.assertEQ(e2.param5, 500);
}

function check_struct_F() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let f: StructExtendTest.F = StructExtendTest.create_F(in_e);
    arktest.assertEQ(StructExtendTest.check_F(f), true);
    let e: StructExtendTest.E = f.barF.getE();
    arktest.assertEQ(e.param4, 4);
    arktest.assertEQ(e.param5, 5);
}

function check_struct_G() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let g: StructExtendTest.F = StructExtendTest.create_G(in_e);
    arktest.assertEQ(StructExtendTest.check_F(g), true);
    let e1: StructExtendTest.E = g.barF.getE();
    arktest.assertEQ(e1.param4, 4);
    arktest.assertEQ(e1.param5, 5);
    let e2: StructExtendTest.E = g.barG.getE();
    arktest.assertEQ(e2.param4, 4);
    arktest.assertEQ(e2.param5, 5);
}

function check_class_H() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let h: StructExtendTest.H = StructExtendTest.create_H(in_e);
    arktest.assertEQ(StructExtendTest.check_H(h), true);
    let e1: StructExtendTest.E = h.barF.getE();
    arktest.assertEQ(e1.param4, 4);
    arktest.assertEQ(e1.param5, 5);
    let e2: StructExtendTest.E = h.barG.getE();
    arktest.assertEQ(e2.param4, 4);
    arktest.assertEQ(e2.param5, 5);
    let e3: StructExtendTest.E = h.barH.getE();
    arktest.assertEQ(e3.param4, 4);
    arktest.assertEQ(e3.param5, 5);
}

function check_class_I() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let i: StructExtendTest.I = StructExtendTest.create_I(in_e);
    arktest.assertEQ(StructExtendTest.check_I(i), true);
    let e1: StructExtendTest.E = i.barF.getE();
    arktest.assertEQ(e1.param4, 4);
    arktest.assertEQ(e1.param5, 5);
    let e2: StructExtendTest.E = i.barG.getE();
    arktest.assertEQ(e2.param4, 4);
    arktest.assertEQ(e2.param5, 5);
    let e3: StructExtendTest.E = i.barH.getE();
    arktest.assertEQ(e3.param4, 4);
    arktest.assertEQ(e3.param5, 5);
    let e4: StructExtendTest.E = i.barI.getE();
    arktest.assertEQ(e4.param4, 4);
    arktest.assertEQ(e4.param5, 5);
}

function check_class_J() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let j: StructExtendTest.J = StructExtendTest.create_J(in_e);
    arktest.assertEQ(StructExtendTest.check_J(j), true);
    let e1: StructExtendTest.E = j.barF.getE();
    arktest.assertEQ(e1.param4, 4);
    arktest.assertEQ(e1.param5, 5);
    let e2: StructExtendTest.E = j.barG.getE();
    arktest.assertEQ(e2.param4, 4);
    arktest.assertEQ(e2.param5, 5);
    let e3: StructExtendTest.E = j.barH.getE();
    arktest.assertEQ(e3.param4, 4);
    arktest.assertEQ(e3.param5, 5);
    let e4: StructExtendTest.E = j.barI.getE();
    arktest.assertEQ(e4.param4, 4);
    arktest.assertEQ(e4.param5, 5);
    let e5: StructExtendTest.E = j.barJ.getE();
    arktest.assertEQ(e5.param4, 4);
    arktest.assertEQ(e5.param5, 5);
}

function main() {
    const suite = new arktest.ArkTestsuite("struct extend ut")
    suite.addTest("check struct A", check_struct_A)
    suite.addTest("check struct B", check_struct_B)
    suite.addTest("check struct C", check_struct_C)
    suite.addTest("check struct D", check_struct_D)
    suite.addTest("check struct E", check_struct_E)
    suite.addTest("check class bar", check_class_Bar)
    suite.addTest("check struct F", check_struct_F)
    suite.addTest("check struct G", check_struct_G)
    suite.addTest("check class H", check_class_H)
    suite.addTest("check class I", check_class_I)
    suite.addTest("check class J", check_class_J)
    exit(suite.run())
}
