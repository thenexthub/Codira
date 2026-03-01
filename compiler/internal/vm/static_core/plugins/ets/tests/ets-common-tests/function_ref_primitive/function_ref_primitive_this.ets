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

// Int
let tmpInt: Int = 0;
let arrInt = new Array<Int>(1, 2, 3, 4);
let okInt: Boolean = true;
let idxInt: Int = 0;
for (tmpInt of arrInt) {
    let f = tmpInt.intValue;
    let r = f();
    if (r != arrInt[idxInt]) okInt = false;
    idxInt++;
}
arktest.assertEQ(okInt, true);

// Long
let tmpLong: Long = 0 as Long;
let arrLong = new Array<Long>(10 as Long, 20 as Long, 30 as Long, 40 as Long);
let okLong: Boolean = true;
let idxLong: Int = 0;
for (tmpLong of arrLong) {
    let f = tmpLong.toLong;
    let r = f();
    if (r != arrLong[idxLong]) okLong = false;
    idxLong++;
}
arktest.assertEQ(okLong, true);

// Double
let tmpDouble: Double = 0.0;
let arrDouble = new Array<Double>(1.1, 2.2, 3.3, 4.4);
let okDouble: Boolean = true;
let idxDouble: Int = 0;
for (tmpDouble of arrDouble) {
    let f = tmpDouble.toDouble;
    let r = f();
    if (r != arrDouble[idxDouble]) okDouble = false;
    idxDouble++;
}
arktest.assertEQ(okDouble, true);

// Float
let tmpFloat: Float = Double.toFloat(0.0);
let arrFloat = new Array<Float>(Double.toFloat(1.5), Double.toFloat(2.5), Double.toFloat(3.5), Double.toFloat(4.5));
let okFloat: Boolean = true;
let idxFloat: Int = 0;
for (tmpFloat of arrFloat) {
    let f = tmpFloat.toFloat;
    let r = f();
    if (r != arrFloat[idxFloat]) okFloat = false;
    idxFloat++;
}
arktest.assertEQ(okFloat, true);

// Byte
let tmpByte: Byte = 0 as Byte;
let arrByte = new Array<Byte>(1 as Byte, 2 as Byte, 3 as Byte, 4 as Byte);
let okByte: Boolean = true;
let idxByte: Int = 0;
for (tmpByte of arrByte) {
    let f = tmpByte.toByte;
    let r = f();
    if (r != arrByte[idxByte]) okByte = false;
    idxByte++;
}
arktest.assertEQ(okByte, true);

// Short
let tmpShort: Short = 0 as Short;
let arrShort = new Array<Short>(100 as Short, 200 as Short, 300 as Short, 400 as Short);
let okShort: Boolean = true;
let idxShort: Int = 0;
for (tmpShort of arrShort) {
    let f = tmpShort.toShort;
    let r = f();
    if (r != arrShort[idxShort]) okShort = false;
    idxShort++;
}
arktest.assertEQ(okShort, true);

// Char
let tmpChar: Char = new Char(c'A');
let arrChar = new Array<Char>(c'A', c'B', c'C', c'D');
let okChar: Boolean = true;
let idxChar: Int = 0;
for (tmpChar of arrChar) {
    let f = tmpChar.toChar;
    let r = f();
    if (r != (arrChar[idxChar].toChar)()) {
        okChar = false;
    }
    idxChar++;
}
arktest.assertEQ(okChar, true);

// Boolean
let tmpBool: Boolean = false;
let arrBool = new Array<Boolean>(true, false, true);
let okBool: Boolean = true;
let idxBool: Int = 0;
for (tmpBool of arrBool) {
    let f = tmpBool.toBoolean;
    let r = f();
    if (r != arrBool[idxBool]) okBool = false;
    idxBool++;
}
arktest.assertEQ(okBool, true);

// equal tests
class A {
    foo() {}
}

class B extends A {
    foo() {}
}

function cmp(a: Any, b: Any) {
    return a == b;
}

function test(a: A, b: B) {
    arktest.assertEQ(cmp(a.foo, b.foo), true);
}

let v = new B();
test(v, v);


// Virtual override resolution
class VBase { foo(): Int { return 1; } }
class VSub extends VBase { override foo(): Int { return 1; } }
let vb = new VSub();
let va: VBase = vb;
arktest.assertEQ(va.foo == vb.foo, true);
let vb2 = new VSub();
arktest.assertEQ(vb.foo == vb2.foo, false);
let vbaseOnly = new VBase();
arktest.assertEQ(va.foo == vbaseOnly.foo, false);

// Interface method comparison
interface IFace { foo(): Int }
class IFImpl1 implements IFace { foo(): Int { return 7; } }
class IFImpl2 implements IFace { foo(): Int { return 8; } }
let iobj1 = new IFImpl1();
let iface1: IFace = iobj1;
arktest.assertEQ(iface1.foo == iobj1.foo, true);
let iobj1b = new IFImpl1();
arktest.assertEQ(iobj1.foo == iobj1b.foo, false);
let iobj2 = new IFImpl2();
arktest.assertEQ(iobj1.foo == iobj2.foo, false);


// Non-virtual (final) method comparison
class WithFinal {
    final baz(): Int { return 42; }
    getBaz() { return this.baz; }
}
let f1 = new WithFinal();
let f2 = f1;
arktest.assertEQ(f1.getBaz() == f2.getBaz(), true);
let f3 = new WithFinal();
arktest.assertEQ(f1.getBaz() == f3.getBaz(), false);
