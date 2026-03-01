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

export class ByteWrapper {
    constructor(value: byte) {
        this.value = value;
    }

    public value: byte;
}

export class ShortWrapper {
    constructor(value: short) {
        this.value = value;
    }

    public value: short;
}

export class IntWrapper {
    constructor(value: int) {
        this.value = value;
    }

    public value: int;
}

export class LongWrapper {
    constructor(value: long) {
        this.value = value;
    }

    public value: long;
}

export class FloatWrapper {
    constructor(value: float) {
        this.value = value;
    }

    public value: float;
}

export class DoubleWrapper {
    constructor(value: double) {
        this.value = value;
    }

    public value: double;
}

class TestResult {
    constructor(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                    i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                    f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                    f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double) {
        this.i0 = i0;
        this.i1 = i1;
        this.i2 = i2;
        this.i3 = i3;
        this.i4 = i4;
        this.i5 = i5;
        this.i6 = i6;
        this.i7 = i7;
        this.i8 = i8;
        this.i9 = i9;
        this.i10 = i10;
        this.i11 = i11;
        this.i12 = i12;
        this.i13 = i13;
        this.i14 = i14;
        this.i15 = i15;

        this.f0 = f0;
        this.f1 = f1;
        this.f2 = f2;
        this.f3 = f3;
        this.f4 = f4;
        this.f5 = f5;
        this.f6 = f6;
        this.f7 = f7;
        this.f8 = f8;
        this.f9 = f9;
        this.f10 = f10;
        this.f11 = f11;
        this.f12 = f12;
        this.f13 = f13;
        this.f14 = f14;
        this.f15 = f15;
    }

    public check(): int {
        if (this.res.value != 0) {
            return -1;
        }

        if (this.i0.value != 0 as byte) {
            return 1;
        }
        if (this.i1 != 1 as byte) {
            return 2;
        }
        if (this.i2.value != 2 as short) {
            return 3;
        }
        if (this.i3 != 3 as short) {
            return 4;
        }
        if (this.i4.value != 4 as int) {
            return 5;
        }
        if (this.i5 != 5 as int) {
            return 6;
        }
        if (this.i6.value != 6 as long) {
            return 7;
        }
        if (this.i7 != 7 as long) {
            return 8;
        }
        if (this.i8.value != 8 as byte) {
            return 9;
        }
        if (this.i9 != 9 as byte) {
            return 10;
        }
        if (this.i10.value != 10 as short) {
            return 11;
        }
        if (this.i11 != 11 as short) {
            return 12;
        }
        if (this.i12.value != 12 as int) {
            return 13;
        }
        if (this.i13 != 13 as int) {
            return 14;
        }
        if (this.i14.value != 14 as long) {
            return 15;
        }
        if (this.i15 != 15 as long) {
            return 16;
        }

        if (this.f0.value != 0.0f) {
            return 17;
        }
        if (this.f1 != 1.0f) {
            return 18;
        }
        if (this.f2.value != 2.0 as double) {
            return 19;
        }
        if (this.f3 != 3.0 as double) {
            return 20;
        }
        if (this.f4.value != 4.0f) {
            return 21;
        }
        if (this.f5 != 5.0f) {
            return 22;
        }
        if (this.f6.value != 6.0 as double) {
            return 23;
        }
        if (this.f7 != 7.0 as double) {
            return 24;
        }
        if (this.f8.value != 8.0f) {
            return 25;
        }
        if (this.f9 != 9.0f) {
            return 26;
        }
        if (this.f10.value != 10.0 as double) {
            return 27;
        }
        if (this.f11 != 11.0 as double) {
            return 28;
        }
        if (this.f12.value != 12.0f) {
            return 29;
        }
        if (this.f13 != 13.0f) {
            return 30;
        }
        if (this.f14.value != 14.0 as double) {
            return 31;
        }
        if (this.f15 != 15.0 as double) {
            return 32;
        }

        return 0;
    }

    public i0: ByteWrapper;
    public i1: byte;
    public i2: ShortWrapper;
    public i3: short;
    public i4: IntWrapper;
    public i5: int;
    public i6: LongWrapper;
    public i7: long;
    public i8: ByteWrapper;
    public i9: byte;
    public i10: ShortWrapper;
    public i11: short;
    public i12: IntWrapper;
    public i13: int;
    public i14: LongWrapper;
    public i15: long;
    public f0: FloatWrapper;
    public f1: float;
    public f2: DoubleWrapper;
    public f3: double;
    public f4: FloatWrapper;
    public f5: float;
    public f6: DoubleWrapper;
    public f7: double;
    public f8: FloatWrapper;
    public f9: float;
    public f10: DoubleWrapper;
    public f11: double;
    public f12: FloatWrapper;
    public f13: float;
    public f14: DoubleWrapper;
    public f15: double;
    public res: IntWrapper = new IntWrapper(-1);
}

export class NativeModule {
    static {
        loadLibrary("checked_test_quick_native");
    }

    constructor() {
        this.magic = 42378;
    }

    static native generalStaticPrim(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                              i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                              f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                              f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double): int;

    @ani.unsafe.Quick
    static native quickStaticPrim(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                              i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                              f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                              f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double): int;

    static native generalStaticObj(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                                 i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                                 f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                                 f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double): IntWrapper;

    @ani.unsafe.Quick
    static native quickStaticObj(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                                 i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                                 f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                                 f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double): IntWrapper;

    final native generalVirtualFinalPrim(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                                   i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                                   f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                                   f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double): int;

    @ani.unsafe.Quick
    final native quickVirtualFinalPrim(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                                   i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                                   f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                                   f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double): int;

    final native generalVirtualFinalObj(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                                      i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                                      f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                                      f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double): IntWrapper;

    @ani.unsafe.Quick
    final native quickVirtualFinalObj(i0: ByteWrapper, i1: byte, i2: ShortWrapper, i3: short, i4: IntWrapper, i5: int, i6: LongWrapper, i7: long,
                                      i8: ByteWrapper, i9: byte, i10: ShortWrapper, i11: short, i12: IntWrapper, i13: int, i14: LongWrapper, i15: long,
                                      f0: FloatWrapper, f1: float, f2: DoubleWrapper, f3: double, f4: FloatWrapper, f5: float, f6: DoubleWrapper, f7: double,
                                      f8: FloatWrapper, f9: float, f10: DoubleWrapper, f11: double, f12: FloatWrapper, f13: float, f14: DoubleWrapper, f15: double): IntWrapper;

    public static classMagic: int = 27384;
    public magic: int;
}

function testGeneralStaticPrim(): TestResult {
    let tr = new TestResult(new ByteWrapper(0 as byte), 1 as byte, new ShortWrapper(2 as short), 3 as short, new IntWrapper(4 as int), 5 as int, new LongWrapper(6 as long), 7 as long,
                            new ByteWrapper(8 as byte), 9 as byte, new ShortWrapper(10 as short), 11 as short, new IntWrapper(12 as int), 13 as int, new LongWrapper(14 as long), 15 as long,
                            new FloatWrapper(0.0f), 1.0f, new DoubleWrapper(2.0 as double), 3.0 as double, new FloatWrapper(4.0f), 5.0f, new DoubleWrapper(6.0 as double), 7.0 as double,
                            new FloatWrapper(8.0f), 9.0f, new DoubleWrapper(10.0 as double), 11.0 as double, new FloatWrapper(12.0f), 13.0f, new DoubleWrapper(14.0 as double), 15.0 as double);
    let res = NativeModule.generalStaticPrim(tr.i0, tr.i1, tr.i2, tr.i3, tr.i4, tr.i5, tr.i6, tr.i7, tr.i8, tr.i9, tr.i10, tr.i11, tr.i12, tr.i13, tr.i14, tr.i15,
                                          tr.f0, tr.f1, tr.f2, tr.f3, tr.f4, tr.f5, tr.f6, tr.f7, tr.f8, tr.f9, tr.f10, tr.f11, tr.f12, tr.f13, tr.f14, tr.f15);
    tr.res = new IntWrapper(res);
    return tr;
}

function testQuickStaticPrim(): TestResult {
    let tr = new TestResult(new ByteWrapper(0 as byte), 1 as byte, new ShortWrapper(2 as short), 3 as short, new IntWrapper(4 as int), 5 as int, new LongWrapper(6 as long), 7 as long,
                            new ByteWrapper(8 as byte), 9 as byte, new ShortWrapper(10 as short), 11 as short, new IntWrapper(12 as int), 13 as int, new LongWrapper(14 as long), 15 as long,
                            new FloatWrapper(0.0f), 1.0f, new DoubleWrapper(2.0 as double), 3.0 as double, new FloatWrapper(4.0f), 5.0f, new DoubleWrapper(6.0 as double), 7.0 as double,
                            new FloatWrapper(8.0f), 9.0f, new DoubleWrapper(10.0 as double), 11.0 as double, new FloatWrapper(12.0f), 13.0f, new DoubleWrapper(14.0 as double), 15.0 as double);
    let res = NativeModule.quickStaticPrim(tr.i0, tr.i1, tr.i2, tr.i3, tr.i4, tr.i5, tr.i6, tr.i7, tr.i8, tr.i9, tr.i10, tr.i11, tr.i12, tr.i13, tr.i14, tr.i15,
                                          tr.f0, tr.f1, tr.f2, tr.f3, tr.f4, tr.f5, tr.f6, tr.f7, tr.f8, tr.f9, tr.f10, tr.f11, tr.f12, tr.f13, tr.f14, tr.f15);
    tr.res = new IntWrapper(res);
    return tr;
}

function testGeneralStaticObj(): TestResult {
    let tr = new TestResult(new ByteWrapper(0 as byte), 1 as byte, new ShortWrapper(2 as short), 3 as short, new IntWrapper(4 as int), 5 as int, new LongWrapper(6 as long), 7 as long,
                            new ByteWrapper(8 as byte), 9 as byte, new ShortWrapper(10 as short), 11 as short, new IntWrapper(12 as int), 13 as int, new LongWrapper(14 as long), 15 as long,
                            new FloatWrapper(0.0f), 1.0f, new DoubleWrapper(2.0 as double), 3.0 as double, new FloatWrapper(4.0f), 5.0f, new DoubleWrapper(6.0 as double), 7.0 as double,
                            new FloatWrapper(8.0f), 9.0f, new DoubleWrapper(10.0 as double), 11.0 as double, new FloatWrapper(12.0f), 13.0f, new DoubleWrapper(14.0 as double), 15.0 as double);
    let res = NativeModule.generalStaticObj(tr.i0, tr.i1, tr.i2, tr.i3, tr.i4, tr.i5, tr.i6, tr.i7, tr.i8, tr.i9, tr.i10, tr.i11, tr.i12, tr.i13, tr.i14, tr.i15,
                                          tr.f0, tr.f1, tr.f2, tr.f3, tr.f4, tr.f5, tr.f6, tr.f7, tr.f8, tr.f9, tr.f10, tr.f11, tr.f12, tr.f13, tr.f14, tr.f15);
    tr.res = res;
    return tr;
}

function testQuickStaticObj(): TestResult {
    let tr = new TestResult(new ByteWrapper(0 as byte), 1 as byte, new ShortWrapper(2 as short), 3 as short, new IntWrapper(4 as int), 5 as int, new LongWrapper(6 as long), 7 as long,
                            new ByteWrapper(8 as byte), 9 as byte, new ShortWrapper(10 as short), 11 as short, new IntWrapper(12 as int), 13 as int, new LongWrapper(14 as long), 15 as long,
                            new FloatWrapper(0.0f), 1.0f, new DoubleWrapper(2.0 as double), 3.0 as double, new FloatWrapper(4.0f), 5.0f, new DoubleWrapper(6.0 as double), 7.0 as double,
                            new FloatWrapper(8.0f), 9.0f, new DoubleWrapper(10.0 as double), 11.0 as double, new FloatWrapper(12.0f), 13.0f, new DoubleWrapper(14.0 as double), 15.0 as double);
    let res = NativeModule.quickStaticObj(tr.i0, tr.i1, tr.i2, tr.i3, tr.i4, tr.i5, tr.i6, tr.i7, tr.i8, tr.i9, tr.i10, tr.i11, tr.i12, tr.i13, tr.i14, tr.i15,
                                          tr.f0, tr.f1, tr.f2, tr.f3, tr.f4, tr.f5, tr.f6, tr.f7, tr.f8, tr.f9, tr.f10, tr.f11, tr.f12, tr.f13, tr.f14, tr.f15);
    tr.res = res;
    return tr;
}

function testGeneralVirtualFinalPrim(): TestResult {
    let tr = new TestResult(new ByteWrapper(0 as byte), 1 as byte, new ShortWrapper(2 as short), 3 as short, new IntWrapper(4 as int), 5 as int, new LongWrapper(6 as long), 7 as long,
                            new ByteWrapper(8 as byte), 9 as byte, new ShortWrapper(10 as short), 11 as short, new IntWrapper(12 as int), 13 as int, new LongWrapper(14 as long), 15 as long,
                            new FloatWrapper(0.0f), 1.0f, new DoubleWrapper(2.0 as double), 3.0 as double, new FloatWrapper(4.0f), 5.0f, new DoubleWrapper(6.0 as double), 7.0 as double,
                            new FloatWrapper(8.0f), 9.0f, new DoubleWrapper(10.0 as double), 11.0 as double, new FloatWrapper(12.0f), 13.0f, new DoubleWrapper(14.0 as double), 15.0 as double);
    let res = (new NativeModule).generalVirtualFinalPrim(tr.i0, tr.i1, tr.i2, tr.i3, tr.i4, tr.i5, tr.i6, tr.i7, tr.i8, tr.i9, tr.i10, tr.i11, tr.i12, tr.i13, tr.i14, tr.i15,
                                          tr.f0, tr.f1, tr.f2, tr.f3, tr.f4, tr.f5, tr.f6, tr.f7, tr.f8, tr.f9, tr.f10, tr.f11, tr.f12, tr.f13, tr.f14, tr.f15);
    tr.res = new IntWrapper(res);
    return tr;
}

function testQuickVirtualFinalPrim(): TestResult {
    let tr = new TestResult(new ByteWrapper(0 as byte), 1 as byte, new ShortWrapper(2 as short), 3 as short, new IntWrapper(4 as int), 5 as int, new LongWrapper(6 as long), 7 as long,
                            new ByteWrapper(8 as byte), 9 as byte, new ShortWrapper(10 as short), 11 as short, new IntWrapper(12 as int), 13 as int, new LongWrapper(14 as long), 15 as long,
                            new FloatWrapper(0.0f), 1.0f, new DoubleWrapper(2.0 as double), 3.0 as double, new FloatWrapper(4.0f), 5.0f, new DoubleWrapper(6.0 as double), 7.0 as double,
                            new FloatWrapper(8.0f), 9.0f, new DoubleWrapper(10.0 as double), 11.0 as double, new FloatWrapper(12.0f), 13.0f, new DoubleWrapper(14.0 as double), 15.0 as double);
    let res = (new NativeModule).quickVirtualFinalPrim(tr.i0, tr.i1, tr.i2, tr.i3, tr.i4, tr.i5, tr.i6, tr.i7, tr.i8, tr.i9, tr.i10, tr.i11, tr.i12, tr.i13, tr.i14, tr.i15,
                                          tr.f0, tr.f1, tr.f2, tr.f3, tr.f4, tr.f5, tr.f6, tr.f7, tr.f8, tr.f9, tr.f10, tr.f11, tr.f12, tr.f13, tr.f14, tr.f15);
    tr.res = new IntWrapper(res);
    return tr;
}

function testGeneralVirtualFinalObj(): TestResult {
    let tr = new TestResult(new ByteWrapper(0 as byte), 1 as byte, new ShortWrapper(2 as short), 3 as short, new IntWrapper(4 as int), 5 as int, new LongWrapper(6 as long), 7 as long,
                            new ByteWrapper(8 as byte), 9 as byte, new ShortWrapper(10 as short), 11 as short, new IntWrapper(12 as int), 13 as int, new LongWrapper(14 as long), 15 as long,
                            new FloatWrapper(0.0f), 1.0f, new DoubleWrapper(2.0 as double), 3.0 as double, new FloatWrapper(4.0f), 5.0f, new DoubleWrapper(6.0 as double), 7.0 as double,
                            new FloatWrapper(8.0f), 9.0f, new DoubleWrapper(10.0 as double), 11.0 as double, new FloatWrapper(12.0f), 13.0f, new DoubleWrapper(14.0 as double), 15.0 as double);
    let res = (new NativeModule).generalVirtualFinalObj(tr.i0, tr.i1, tr.i2, tr.i3, tr.i4, tr.i5, tr.i6, tr.i7, tr.i8, tr.i9, tr.i10, tr.i11, tr.i12, tr.i13, tr.i14, tr.i15,
                                          tr.f0, tr.f1, tr.f2, tr.f3, tr.f4, tr.f5, tr.f6, tr.f7, tr.f8, tr.f9, tr.f10, tr.f11, tr.f12, tr.f13, tr.f14, tr.f15);
    tr.res = res;
    return tr;
}

function testQuickVirtualFinalObj(): TestResult {
    let tr = new TestResult(new ByteWrapper(0 as byte), 1 as byte, new ShortWrapper(2 as short), 3 as short, new IntWrapper(4 as int), 5 as int, new LongWrapper(6 as long), 7 as long,
                            new ByteWrapper(8 as byte), 9 as byte, new ShortWrapper(10 as short), 11 as short, new IntWrapper(12 as int), 13 as int, new LongWrapper(14 as long), 15 as long,
                            new FloatWrapper(0.0f), 1.0f, new DoubleWrapper(2.0 as double), 3.0 as double, new FloatWrapper(4.0f), 5.0f, new DoubleWrapper(6.0 as double), 7.0 as double,
                            new FloatWrapper(8.0f), 9.0f, new DoubleWrapper(10.0 as double), 11.0 as double, new FloatWrapper(12.0f), 13.0f, new DoubleWrapper(14.0 as double), 15.0 as double);
    let res = (new NativeModule).quickVirtualFinalObj(tr.i0, tr.i1, tr.i2, tr.i3, tr.i4, tr.i5, tr.i6, tr.i7, tr.i8, tr.i9, tr.i10, tr.i11, tr.i12, tr.i13, tr.i14, tr.i15,
                                          tr.f0, tr.f1, tr.f2, tr.f3, tr.f4, tr.f5, tr.f6, tr.f7, tr.f8, tr.f9, tr.f10, tr.f11, tr.f12, tr.f13, tr.f14, tr.f15);
    tr.res = res;
    return tr;
}

//! DISABLED_CHECKER       General ANI call optimization, JIT
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-hotness-threshold=1 --compiler-regex='quick_native.ETSGLOBAL::test.*'", entry: "quick_native.ETSGLOBAL::runGeneralNative"
//! METHOD        "quick_native.ETSGLOBAL::testGeneralStaticPrim"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Static/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /CallStatic\.Inlined.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NEXT     /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NEXT     /WrapObjectNative/
//! INST_NEXT     /CallNative.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NEXT     /ReturnInlined/
//! INST_NEXT     /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testGeneralStaticObj"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Static/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /CallStatic\.Inlined.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NEXT     /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NEXT     /WrapObjectNative/
//! INST_NEXT     /CallNative.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NEXT     /ReturnInlined/
//! INST_NEXT     /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testGeneralVirtualFinalPrim"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Virtual/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /CallStatic\.Inlined.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NEXT     /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NEXT     /WrapObjectNative/
//! INST_NEXT     /CallNative.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NEXT     /ReturnInlined/
//! INST_NEXT     /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testGeneralVirtualFinalObj"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Virtual/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /CallStatic\.Inlined.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NEXT     /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NEXT     /WrapObjectNative/
//! INST_NEXT     /CallNative.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NEXT     /ReturnInlined/
//! INST_NEXT     /Intrinsic\.CompilerCheckNativeException/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testGeneralStaticPrim,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testGeneralStaticObj,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testGeneralVirtualFinalPrim,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testGeneralVirtualFinalObj,NOT_SUPPORTED_NATIVE/

//! CHECKER       Quick ANI call optimization, JIT
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-hotness-threshold=1 --compiler-regex='quick_native.ETSGLOBAL::test.*'", entry: "quick_native.ETSGLOBAL::runQuickNative"
//! METHOD        "quick_native.ETSGLOBAL::testQuickStaticPrim"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Static/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST_NOT      /CallStatic\.Inlined.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testQuickStaticObj"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Static/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST_NOT      /CallStatic\.Inlined.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testQuickVirtualFinalPrim"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Virtual/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST          /CallStatic\.Inlined.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testQuickVirtualFinalObj"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Virtual/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST          /CallStatic\.Inlined.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testQuickStaticPrim,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testQuickStaticObj,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testQuickVirtualFinalPrim,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testQuickVirtualFinalObj,NOT_SUPPORTED_NATIVE/

//! DISABLED_CHECKER       General ANI call optimization, AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='quick_native.ETSGLOBAL::test.*'"
//! METHOD        "quick_native.ETSGLOBAL::testGeneralStaticPrim"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Static/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /CallStatic\.Inlined.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NEXT     /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NEXT     /WrapObjectNative/
//! INST_NEXT     /CallNative.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NEXT     /ReturnInlined/
//! INST_NEXT     /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testGeneralStaticObj"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Static/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /CallStatic\.Inlined.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NEXT     /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NEXT     /WrapObjectNative/
//! INST_NEXT     /CallNative.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NEXT     /ReturnInlined/
//! INST_NEXT     /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testGeneralVirtualFinalPrim"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Virtual/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /CallStatic\.Inlined.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NEXT     /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NEXT     /WrapObjectNative/
//! INST_NEXT     /CallNative.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NEXT     /ReturnInlined/
//! INST_NEXT     /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testGeneralVirtualFinalObj"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Virtual/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /CallStatic\.Inlined.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NEXT     /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NEXT     /WrapObjectNative/
//! INST_NEXT     /CallNative.*NativeModule/
//! INST_NEXT     /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NEXT     /ReturnInlined/
//! INST_NEXT     /Intrinsic\.CompilerCheckNativeException/
//! RUN           entry: "quick_native.ETSGLOBAL::runGeneralNative"
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testGeneralStaticPrim,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testGeneralStaticObj,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testGeneralVirtualFinalPrim,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testGeneralVirtualFinalObj,NOT_SUPPORTED_NATIVE/

//! CHECKER       Quick ANI call optimization, AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='quick_native.ETSGLOBAL::test.*'"
//! METHOD        "quick_native.ETSGLOBAL::testQuickStaticPrim"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Static/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST_NOT      /CallStatic\.Inlined.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testQuickStaticObj"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Static/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST_NOT      /CallStatic\.Inlined.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testQuickVirtualFinalPrim"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Virtual/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST          /CallStatic\.Inlined.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testQuickVirtualFinalObj"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*NativeModule.*Virtual/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST          /CallStatic\.Inlined.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*NativeModule/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! RUN           entry: "quick_native.ETSGLOBAL::runQuickNative"
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testQuickStaticPrim,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testQuickStaticObj,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testQuickVirtualFinalPrim,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testQuickVirtualFinalObj,NOT_SUPPORTED_NATIVE/

function runGeneralNative(): int {
    let res = 0;
    for (let i = 0; i < 100; ++i) {
        res = testGeneralStaticPrim().check();
        if (res != 0) {
            console.log('testGeneralStaticPrim failed!');
            return res;
        }

        res = testGeneralStaticObj().check();
        if (res != 0) {
            console.log('testGeneralStaticObj failed!');
            return res;
        }

        res = testGeneralVirtualFinalPrim().check();
        if (res != 0) {
            console.log('testGeneralVirtualFinalPrim failed!');
            return res;
        }

        res = testGeneralVirtualFinalObj().check();
        if (res != 0) {
            console.log('testGeneralVirtualFinalObj failed!');
            return res;
        }
    }
    return res;
}

function runQuickNative(): int {
    let res = 0;
    for (let i = 0; i < 100; ++i) {
        res = testQuickStaticPrim().check();
        if (res != 0) {
            console.log('testQuickStaticPrim failed!');
            return res;
        }

        res = testQuickStaticObj().check();
        if (res != 0) {
            console.log('testQuickStaticObj failed!');
            return res;
        }

        res = testQuickVirtualFinalPrim().check();
        if (res != 0) {
            console.log('testQuickVirtualFinalPrim failed!');
            return res;
        }

        res = testQuickVirtualFinalObj().check();
        if (res != 0) {
            console.log('testQuickVirtualFinalObj failed!');
            return res;
        }
    }
    return res;
}

native function generalModuleLevelFunc(i0: int): int;

@ani.unsafe.Quick
native function quickModuleLevelFunc(i0: int): int;

function testNativeQuick(): boolean {
    return quickModuleLevelFunc(0) == 0;
}

function testNativeGeneral(): boolean {
    return generalModuleLevelFunc(0) == 0;
}

//! CHECKER       ANI module call optimization failure, JIT
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options:  "--compiler-hotness-threshold=0 --compiler-regex='quick_native.ETSGLOBAL::testNative.*'", entry: "quick_native.ETSGLOBAL::runModuleLevelTest"
//! METHOD        "quick_native.ETSGLOBAL::testNativeQuick"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*quickModuleLevelFunc.*/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST_NOT      /CallStatic\.Inlined.*quickModuleLevelFunc/
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*quickModuleLevelFunc/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodPrim/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! METHOD        "quick_native.ETSGLOBAL::testNativeGeneral"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic.*generalModuleLevelFunc.*/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethod/
//! INST_NOT      /CallStatic\.Inlined.*testNativeGeneral/
//! INST_NOT      /Intrinsic\.CompilerGetNativeMethodManagedClass/
//! INST_NOT      /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NOT      /Intrinsic\.CompilerGetNativeApiEnv/
//! INST_NOT      /Intrinsic\.CompilerBeginNativeMethod/
//! INST_NOT      /WrapObjectNative/
//! INST_NOT      /CallNative.*testNativeGeneral/
//! INST_NOT      /Intrinsic\.CompilerEndNativeMethodObj/
//! INST_NOT      /Intrinsic\.CompilerCheckNativeException/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testNativeQuick,NOT_SUPPORTED_NATIVE/
//! EVENT_NOT     /DeoptimizationReason,quick_native.ETSGLOBAL::testNativeGeneral,NOT_SUPPORTED_NATIVE/

function runModuleLevelTest(): int {

    loadLibrary("checked_test_quick_native");

    if (!testNativeQuick()) {
        console.log('testNativeQuick failed!');
        return 1;
    }

    if (!testNativeGeneral()) {
        console.log('testNativeGeneral failed!');
        return 2;
    }

    return 0;
}
