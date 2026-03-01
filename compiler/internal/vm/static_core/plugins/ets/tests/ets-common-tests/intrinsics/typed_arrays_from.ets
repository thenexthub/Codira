/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function testIU8FromArrayWrongType() {
    let strings: Array<string> = Array.of(new string("hi"), new string("hello"), new string("new"));
    __noinline__Int8ArrayFromCheckException(strings, 'std.core.LineString');
    let numbers: Object = Array.of(1.0, 2.0, 3.0);
    __noinline__Int8ArrayFromCheckException(numbers, 'std.core.Double');
}

function testIU8FromIterableNumber() {
    checkNumberArray(Int8Array.of(), Int8Array.from(new Set<number>()));
    checkNumberArray(Uint8Array.of(), Uint8Array.from(new Set<number>()));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from(new Set<number>()));
    checkNumberArray(Int8Array.of(), Int8Array.from(new Set<int>()));
    checkNumberArray(Uint8Array.of(), Uint8Array.from(new Set<int>()));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from(new Set<int>()));

    const len: int = 127;
    let iexpected: Int8Array = new Int8Array(len);
    let uexpected: Uint8Array = new Uint8Array(len);
    for (let i: int = 0; i < len; i++) {
        iexpected[i] = Int.toDouble(i + 1);
        uexpected[i] = Int.toDouble(i + 1);
    }

    let data = prepareTestData(len, 1);
    let set = data[0];
    checkNumberArray(iexpected, Int8Array.from(set));
    checkNumberArray(uexpected, Uint8Array.from(set));

    checkNumberArray(Int8Array.of(0, 1, -1, -1, 1, -1, 0),
        Int8Array.from(new Set<int>(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648))));
    checkNumberArray(Uint8Array.of(0, 1, 255, 255, 1, 255, 0),
        Uint8Array.from(new Set<int>(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648))));
    checkNumberArray(Uint8ClampedArray.of(0, 1, 0, 255, 0, 255, 0),
        Uint8ClampedArray.from(new Set<int>(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648))));

    checkNumberArray(Int8Array.of(1, -1, -126, -127, -128, 127, 0, -1, 0, 12, -1, 126, 127, -128, -2, 0, 1, 16),
        Int8Array.from(new Set<int>(Array.of<int>(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128,
            254, 256, 257, 10000))));
    checkNumberArray(Uint8Array.of(1, 255, 130, 129, 128, 127, 0, 255, 0, 12, 255, 126, 127, 128, 254, 0, 1, 16),
        Uint8Array.from(new Set<int>(Array.of<int>(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128,
            254, 256, 257, 10000))));
    checkNumberArray(Uint8ClampedArray.of(1, 0, 0, 0, 0, 0, 0, 0, 0, 12, 255, 126, 127, 128, 254, 255, 255, 255),
        Uint8ClampedArray.from(new Set<int>(Array.of<int>(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126,
            127, 128, 254, 256, 257, 10000))));
}

function testIU8FromArrayLikeNumber() {
    let empty: FixedArray<int> = [];
    checkNumberArray(Int8Array.of(), Int8Array.from(new Array<int>()));
    checkNumberArray(Int8Array.of(), Int8Array.from([]));
    checkNumberArray(Int8Array.of(), Int8Array.from(empty));
    checkNumberArray(Int8Array.of(), Int8Array.from(Int8Array.of()));
    checkNumberArray(Int8Array.of(), Int8Array.from(Uint8Array.of()));
    checkNumberArray(Int8Array.of(), Int8Array.from(Int32Array.of()));
    checkNumberArray(Int8Array.of(), Int8Array.from(Uint32Array.of()));
    checkNumberArray(Uint8Array.of(), Uint8Array.from(new Array<int>()));
    checkNumberArray(Uint8Array.of(), Uint8Array.from([]));
    checkNumberArray(Uint8Array.of(), Uint8Array.from(empty));
    checkNumberArray(Uint8Array.of(), Uint8Array.from(Uint8Array.of()));
    checkNumberArray(Uint8Array.of(), Uint8Array.from(Int8Array.of()));
    checkNumberArray(Uint8Array.of(), Uint8Array.from(Uint32Array.of()));
    checkNumberArray(Uint8Array.of(), Uint8Array.from(Int32Array.of()));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from(new Array<int>()));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from([]));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from(empty));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from(Uint8Array.of()));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from(Int8Array.of()));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from(Uint32Array.of()));
    checkNumberArray(Uint8ClampedArray.of(), Uint8ClampedArray.from(Int32Array.of()));

    const len: int = 127;
    let iexpected: Int8Array = new Int8Array(len);
    let uexpected: Uint8Array = new Uint8Array(len);
    let i8arr: Int8Array = new Int8Array(len);
    let u8arr: Uint8Array = new Uint8Array(len);
    let i16arr: Int16Array = new Int16Array(len);
    let u16arr: Uint16Array = new Uint16Array(len);
    for (let i: int = 0; i < len; i++) {
        iexpected[i] = i8arr[i] = i16arr[i] = Int.toDouble(i + 1);
        uexpected[i] = u8arr[i] = u16arr[i] = Int.toDouble(i + 1);
    }

    let data = prepareTestData(len, 1);
    let arr = data[1];
    let fixarr = data[2];
    checkNumberArray(iexpected, Int8Array.from(arr));
    checkNumberArray(iexpected, Int8Array.from(fixarr));
    checkNumberArray(iexpected, Int8Array.from(i8arr));
    checkNumberArray(iexpected, Int8Array.from(u8arr));
    checkNumberArray(iexpected, Int8Array.from(i16arr));
    checkNumberArray(iexpected, Int8Array.from(u16arr));
    checkNumberArray(uexpected, Uint8Array.from(arr));
    checkNumberArray(uexpected, Uint8Array.from(fixarr));
    checkNumberArray(uexpected, Uint8Array.from(i8arr));
    checkNumberArray(uexpected, Uint8Array.from(u8arr));
    checkNumberArray(uexpected, Uint8Array.from(i16arr));
    checkNumberArray(uexpected, Uint8Array.from(u16arr));

    let source: FixedArray<int> = [0, 1, -1, 65535, -65535, 2147483647, -2147483648];
    checkNumberArray(Int8Array.of(0, 1, -1, -1, 1, -1, 0),
        Int8Array.from(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648)));
    checkNumberArray(Int8Array.of(0, 1, -1, -1, 1, -1, 0),
        Int8Array.from([0, 1, -1, 65535, -65535, 2147483647, -2147483648]));
    checkNumberArray(Int8Array.of(0, 1, -1, -1, 1, -1, 0), Int8Array.from(source));
    checkNumberArray(Uint8Array.of(0, 1, 255, 255, 1, 255, 0),
        Uint8Array.from(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648)));
    checkNumberArray(Uint8Array.of(0, 1, 255, 255, 1, 255, 0),
        Uint8Array.from([0, 1, -1, 65535, -65535, 2147483647, -2147483648]));
    checkNumberArray(Uint8Array.of(0, 1, 255, 255, 1, 255, 0), Uint8Array.from(source));
    checkNumberArray(Uint8ClampedArray.of(0, 1, 0, 255, 0, 255, 0),
        Uint8ClampedArray.from(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648)));
    checkNumberArray(Uint8ClampedArray.of(0, 1, 0, 255, 0, 255, 0),
        Uint8ClampedArray.from([0, 1, -1, 65535, -65535, 2147483647, -2147483648]));
    checkNumberArray(Uint8ClampedArray.of(0, 1, 0, 255, 0, 255, 0), Uint8ClampedArray.from(source));

    let source2: FixedArray<int> = [1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128,
        254, 256, 257, 10000];
    checkNumberArray(Int8Array.of(1, -1, -126, -127, -128, 127, 0, -1, 0, 12, -1, 126, 127, -128, -2, 0, 1, 16),
        Int8Array.from(Array.of<int>(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128,
            254, 256, 257, 10000)));
    checkNumberArray(Int8Array.of(1, -1, -126, -127, -128, 127, 0, -1, 0, 12, -1, 126, 127, -128, -2, 0, 1, 16),
        Int8Array.from([1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128,
            254, 256, 257, 10000]));
    checkNumberArray(Int8Array.of(1, -1, -126, -127, -128, 127, 0, -1, 0, 12, -1, 126, 127, -128, -2, 0, 1, 16),
        Int8Array.from(source2));
    checkNumberArray(Uint8Array.of(1, 255, 130, 129, 128, 127, 0, 255, 0, 12, 255, 126, 127, 128, 254, 0, 1, 16),
        Uint8Array.from(Array.of<int>(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128,
            254, 256, 257, 10000)));
    checkNumberArray(Uint8Array.of(1, 255, 130, 129, 128, 127, 0, 255, 0, 12, 255, 126, 127, 128, 254, 0, 1, 16),
        Uint8Array.from([1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128,
            254, 256, 257, 10000]));
    checkNumberArray(Uint8Array.of(1, 255, 130, 129, 128, 127, 0, 255, 0, 12, 255, 126, 127, 128, 254, 0, 1, 16),
        Uint8Array.from(source2));
    checkNumberArray(Uint8ClampedArray.of(1, 0, 0, 0, 0, 0, 0, 0, 0, 12, 255, 126, 127, 128, 254, 255, 255, 255),
        Uint8ClampedArray.from(Array.of<int>(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126,
            127, 128, 254, 256, 257, 10000)));
    checkNumberArray(Uint8ClampedArray.of(1, 0, 0, 0, 0, 0, 0, 0, 0, 12, 255, 126, 127, 128, 254, 255, 255, 255),
        Uint8ClampedArray.from([1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126,
            127, 128, 254, 256, 257, 10000]));
    checkNumberArray(Uint8ClampedArray.of(1, 0, 0, 0, 0, 0, 0, 0, 0, 12, 255, 126, 127, 128, 254, 255, 255, 255),
        Uint8ClampedArray.from(source2));
    Uint8Array.from(Int8Array.of(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128, 254, 256, 257, 10000));
}

function testIU16FromIterableNumber() {
    checkNumberArray(Int16Array.of(), Int16Array.from(new Set<int>()));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(new Set<int>()));

    const len: int = 127;
    let iexpected: Int16Array = new Int16Array(len);
    let uexpected: Uint16Array = new Uint16Array(len);
    for (let i: int = 0; i < len; i++) {
        iexpected[i] = Int.toDouble(i + 128);
        uexpected[i] = Int.toDouble(i + 128);
    }

    let data = prepareTestData(len, 128);
    let set = data[0];
    checkNumberArray(iexpected, Int16Array.from(set));
    checkNumberArray(uexpected, Uint16Array.from(set));

    checkNumberArray(Int16Array.of(0, 1, -1, -1, 1, -1, 0),
        Int16Array.from(new Set<int>(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648))));
    checkNumberArray(Uint16Array.of(0, 1, 65535, 65535, 1, 65535, 0),
        Uint16Array.from(new Set<int>(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648))));

    checkNumberArray(Int16Array.of(-1, -32766, -32767, -32768, 32767, 1, 0, -1, 0, 12, 255, 32766, 32765, -32768,
            -32767, -2, -1, 0, -31072, -1, 0),
        Int16Array.from(new Set<int>(Array.of<int>(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648))));
    checkNumberArray(Uint16Array.of(65535, 32770, 32769, 32768, 32767, 1, 0, 65535, 0, 12, 255, 32766, 32765, 32768,
            32769, 65534, 65535, 0, 34464, 65535, 0),
        Uint16Array.from(new Set<int>(Array.of<int>(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648))));
}

function testIU16FromArrayLikeNumber() {
    let empty: FixedArray<int> = [];
    checkNumberArray(Int16Array.of(), Int16Array.from(new Array<int>()));
    checkNumberArray(Int16Array.of(), Int16Array.from([]));
    checkNumberArray(Int16Array.of(), Int16Array.from(empty));
    checkNumberArray(Int16Array.of(), Int16Array.from(Int8Array.of()));
    checkNumberArray(Int16Array.of(), Int16Array.from(Uint8Array.of()));
    checkNumberArray(Int16Array.of(), Int16Array.from(Int16Array.of()));
    checkNumberArray(Int16Array.of(), Int16Array.from(Uint16Array.of()));
    checkNumberArray(Int16Array.of(), Int16Array.from(Int32Array.of()));
    checkNumberArray(Int16Array.of(), Int16Array.from(Uint32Array.of()));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(new Array<int>()));
    checkNumberArray(Uint16Array.of(), Uint16Array.from([]));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(empty));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(Int8Array.of()));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(Uint8Array.of()));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(Uint16Array.of()));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(Int16Array.of()));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(Int32Array.of()));
    checkNumberArray(Uint16Array.of(), Uint16Array.from(Uint32Array.of()));

    const len: int = 127;
    let iexpected: Int16Array = new Int16Array(len);
    let uexpected: Uint16Array = new Uint16Array(len);
    let i8arr: Int8Array = new Int8Array(len);
    let u8arr: Uint8Array = new Uint8Array(len);
    let i16arr: Int16Array = new Int16Array(len);
    let u16arr: Uint16Array = new Uint16Array(len);
    let i32arr: Int32Array = new Int32Array(len);
    let u32arr: Uint32Array = new Uint32Array(len);

    for (let i: int = 0; i < len; i++) {
        iexpected[i] = i8arr[i] = i16arr[i] = i32arr[i] = Int.toDouble(i + 1);
        uexpected[i] = u8arr[i] = u16arr[i] = u32arr[i] = Int.toDouble(i + 1);
    }

    let data = prepareTestData(len, 1);
    let arr = data[1];
    let fixarr = data[2];
    checkNumberArray(iexpected, Int16Array.from(arr));
    checkNumberArray(iexpected, Int16Array.from(fixarr));
    checkNumberArray(iexpected, Int16Array.from(i8arr));
    checkNumberArray(iexpected, Int16Array.from(u8arr));
    checkNumberArray(iexpected, Int16Array.from(i16arr));
    checkNumberArray(iexpected, Int16Array.from(u16arr));
    checkNumberArray(iexpected, Int16Array.from(i32arr));
    checkNumberArray(iexpected, Int16Array.from(u32arr));
    checkNumberArray(uexpected, Uint16Array.from(arr));
    checkNumberArray(uexpected, Uint16Array.from(fixarr));
    checkNumberArray(uexpected, Uint16Array.from(i8arr));
    checkNumberArray(uexpected, Uint16Array.from(u8arr));
    checkNumberArray(uexpected, Uint16Array.from(i16arr));
    checkNumberArray(uexpected, Uint16Array.from(u16arr));
    checkNumberArray(uexpected, Uint16Array.from(i32arr));
    checkNumberArray(uexpected, Uint16Array.from(u32arr));

    for (let i: int = 0; i < len; i++) {
        iexpected[i] = i16arr[i] = i32arr[i] = Int.toDouble(i + 128);
        uexpected[i] = u16arr[i] = u32arr[i] = Int.toDouble(i + 128);
    }

    data = prepareTestData(len, 128);
    arr = data[1];
    fixarr = data[2];
    checkNumberArray(iexpected, Int16Array.from(arr));
    checkNumberArray(iexpected, Int16Array.from(fixarr));
    checkNumberArray(iexpected, Int16Array.from(i16arr));
    checkNumberArray(iexpected, Int16Array.from(u16arr));
    checkNumberArray(iexpected, Int16Array.from(i32arr));
    checkNumberArray(iexpected, Int16Array.from(u32arr));
    checkNumberArray(uexpected, Uint16Array.from(arr));
    checkNumberArray(uexpected, Uint16Array.from(fixarr));
    checkNumberArray(uexpected, Uint16Array.from(i16arr));
    checkNumberArray(uexpected, Uint16Array.from(u16arr));
    checkNumberArray(uexpected, Uint16Array.from(i32arr));
    checkNumberArray(uexpected, Uint16Array.from(u32arr));

    let source: FixedArray<int> = [0, 1, -1, 65535, -65535, 2147483647, -2147483648];
    checkNumberArray(Int16Array.of(0, 1, -1, -1, 1, -1, 0),
        Int16Array.from(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648)));
    checkNumberArray(Int16Array.of(0, 1, -1, -1, 1, -1, 0),
        Int16Array.from([0, 1, -1, 65535, -65535, 2147483647, -2147483648]));
    checkNumberArray(Int16Array.of(0, 1, -1, -1, 1, -1, 0), Int16Array.from(source));
    checkNumberArray(Uint16Array.of(0, 1, 65535, 65535, 1, 65535, 0),
        Uint16Array.from(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648)));
    checkNumberArray(Uint16Array.of(0, 1, 65535, 65535, 1, 65535, 0),
        Uint16Array.from([0, 1, -1, 65535, -65535, 2147483647, -2147483648]));
    checkNumberArray(Uint16Array.of(0, 1, 65535, 65535, 1, 65535, 0), Uint16Array.from(source));

    let source2: FixedArray<int> = [-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
        255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648];
    checkNumberArray(Int16Array.of(-1, -32766, -32767, -32768, 32767, 1, 0, -1, 0, 12, 255, 32766, 32765, -32768,
            -32767, -2, -1, 0, -31072, -1, 0),
        Int16Array.from(Array.of<int>(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648)));
    checkNumberArray(Int16Array.of(-1, -32766, -32767, -32768, 32767, 1, 0, -1, 0, 12, 255, 32766, 32765, -32768,
            -32767, -2, -1, 0, -31072, -1, 0),
        Int16Array.from([-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648]));
    checkNumberArray(Int16Array.of(-1, -32766, -32767, -32768, 32767, 1, 0, -1, 0, 12, 255, 32766, 32765, -32768,
            -32767, -2, -1, 0, -31072, -1, 0), Int16Array.from(source2));
    checkNumberArray(Uint16Array.of(65535, 32770, 32769, 32768, 32767, 1, 0, 65535, 0, 12, 255, 32766, 32765, 32768,
            32769, 65534, 65535, 0, 34464, 65535, 0),
        Uint16Array.from(Array.of<int>(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648)));
    checkNumberArray(Uint16Array.of(65535, 32770, 32769, 32768, 32767, 1, 0, 65535, 0, 12, 255, 32766, 32765, 32768,
            32769, 65534, 65535, 0, 34464, 65535, 0),
        Uint16Array.from([-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648]));
    checkNumberArray(Uint16Array.of(65535, 32770, 32769, 32768, 32767, 1, 0, 65535, 0, 12, 255, 32766, 32765, 32768,
            32769, 65534, 65535, 0, 34464, 65535, 0), Uint16Array.from(source2));
    checkNumberArray(Int16Array.of(-1, -32766, -32767, -32768, 32767, 1, 0, -1, 0, 12, 255, 32766, 32765, -32768, -32767,
            -2, -1, 0, -31072),
        Int16Array.from(Uint16Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766,
            32765, 32768, 32769, 65534, 65535, 65536, 100000)));
    checkNumberArray(Uint16Array.of(65535, 32770, 32769, 32768, 32767, 1, 0, 65535, 0, 12, 255, 32766, 32765, 32768,
            32769, 65534, 65535, 0, 34464),
        Uint16Array.from(Int16Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766,
            32765, 32768, 32769, 65534, 65535, 65536, 100000)));
}

function testIU32FromIterableNumber() {
    checkNumberArray(Int32Array.of(), Int32Array.from(new Set<int>()));
    checkNumberArray(Uint32Array.of(), Uint32Array.from(new Set<int>()));

    const len: int = 127;
    let iexpected: Int32Array = new Int32Array(len);
    let uexpected: Uint32Array = new Uint32Array(len);
    for (let i: int = 0; i < len; i++) {
        iexpected[i] = Int.toDouble(i + 16536);
        uexpected[i] = Int.toDouble(i + 16536);
    }

    let data = prepareTestData(len, 16536);
    let set = data[0];
    checkNumberArray(iexpected, Int32Array.from(set));
    checkNumberArray(uexpected, Uint32Array.from(set));

    checkNumberArray(Int32Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648),
        Int32Array.from(new Set<int>(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648))));
    checkNumberArray(Uint32Array.of(0.0, 1.0, 4294967295.0, 65535.0, 4294901761.0, 2147483647.0, 2147483648.0),
        Uint32Array.from(new Set<int>(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648))));

    checkNumberArray(Int32Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765,
            32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648),
        Int32Array.from(new Set<int>(Array.of<int>(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648))));
    checkNumberArray(Uint32Array.of(4294967295.0, 4294934530.0, 4294934529.0, 4294934528.0, 4294934527.0, 4294901761.0,
            4294901760.0, 4294901759.0, 0.0, 12.0, 255.0, 32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0,
            65536.0, 100000.0, 2147483647.0, 2147483648.0),
        Uint32Array.from(new Set<int>(Array.of<int>(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648))));
}

function testIU32FromArrayLikeNumber() {
    let empty: FixedArray<int> = [];
    checkNumberArray(Int32Array.of(), Int32Array.from(new Array<int>()));
    checkNumberArray(Int32Array.of(), Int32Array.from([]));
    checkNumberArray(Int32Array.of(), Int32Array.from(empty));
    checkNumberArray(Int32Array.of(), Int32Array.from(Int8Array.of()));
    checkNumberArray(Int32Array.of(), Int32Array.from(Uint8Array.of()));
    checkNumberArray(Int32Array.of(), Int32Array.from(Int32Array.of()));
    checkNumberArray(Int32Array.of(), Int32Array.from(Uint32Array.of()));
    checkNumberArray(Uint32Array.of(), Uint32Array.from(new Array<int>()));
    checkNumberArray(Uint32Array.of(), Uint32Array.from([]));
    checkNumberArray(Uint32Array.of(), Uint32Array.from(empty));
    checkNumberArray(Uint32Array.of(), Uint32Array.from(Uint8Array.of()));
    checkNumberArray(Uint32Array.of(), Uint32Array.from(Int8Array.of()));
    checkNumberArray(Uint32Array.of(), Uint32Array.from(Uint32Array.of()));
    checkNumberArray(Uint32Array.of(), Uint32Array.from(Int32Array.of()));

    const len: int = 127;
    let iexpected: Int32Array = new Int32Array(len);
    let uexpected: Uint32Array = new Uint32Array(len);
    let i8arr: Int8Array = new Int8Array(len);
    let u8arr: Uint8Array = new Uint8Array(len);
    let i32arr: Int32Array = new Int32Array(len);
    let u32arr: Uint32Array = new Uint32Array(len);
    for (let i: int = 0; i < len; i++) {
        iexpected[i] = i8arr[i] = i32arr[i] = Int.toDouble(i + 1);
        uexpected[i] = u8arr[i] = u32arr[i] = Int.toDouble(i + 1);
    }

    let data = prepareTestData(len, 1);
    let arr = data[1];
    let fixarr = data[2];
    checkNumberArray(iexpected, Int32Array.from(arr));
    checkNumberArray(iexpected, Int32Array.from(fixarr));
    checkNumberArray(iexpected, Int32Array.from(i8arr));
    checkNumberArray(iexpected, Int32Array.from(i32arr));
    checkNumberArray(iexpected, Int32Array.from(u8arr));
    checkNumberArray(iexpected, Int32Array.from(u32arr));
    checkNumberArray(uexpected, Uint32Array.from(arr));
    checkNumberArray(uexpected, Uint32Array.from(fixarr));
    checkNumberArray(uexpected, Uint32Array.from(u8arr));
    checkNumberArray(uexpected, Uint32Array.from(u32arr));
    checkNumberArray(uexpected, Uint32Array.from(i8arr));
    checkNumberArray(uexpected, Uint32Array.from(i32arr));

    for (let i: int = 0; i < len; i++) {
        iexpected[i] = i32arr[i] = Int.toDouble(i + 16536);
        uexpected[i] = u32arr[i] = Int.toDouble(i + 16536);
    }

    data = prepareTestData(len, 16536);
    arr = data[1];
    fixarr = data[2];
    checkNumberArray(iexpected, Int32Array.from(arr));
    checkNumberArray(iexpected, Int32Array.from(fixarr));
    checkNumberArray(iexpected, Int32Array.from(i32arr));
    checkNumberArray(iexpected, Int32Array.from(u32arr));
    checkNumberArray(uexpected, Uint32Array.from(arr));
    checkNumberArray(uexpected, Uint32Array.from(fixarr));
    checkNumberArray(uexpected, Uint32Array.from(u32arr));
    checkNumberArray(uexpected, Uint32Array.from(i32arr));

    let source: FixedArray<int> = [0, 1, -1, 65535, -65535, 2147483647, -2147483648];
    checkNumberArray(Int32Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648),
        Int32Array.from(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648)));
    checkNumberArray(Int32Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648),
        Int32Array.from([0, 1, -1, 65535, -65535, 2147483647, -2147483648]));
    checkNumberArray(Int32Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648), Int32Array.from(source));
    checkNumberArray(Uint32Array.of(0.0, 1.0, 4294967295.0, 65535.0, 4294901761.0, 2147483647.0, 2147483648.0),
        Uint32Array.from(Array.of<int>(0, 1, -1, 65535, -65535, 2147483647, -2147483648)));
    checkNumberArray(Uint32Array.of(0.0, 1.0, 4294967295.0, 65535.0, 4294901761.0, 2147483647.0, 2147483648.0),
        Uint32Array.from([0, 1, -1, 65535, -65535, 2147483647, -2147483648]));
    checkNumberArray(Uint32Array.of(0.0, 1.0, 4294967295.0, 65535.0, 4294901761.0, 2147483647.0, 2147483648.0),
        Uint32Array.from(source));

    let source2: FixedArray<int> = [-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
        255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648];
    checkNumberArray(Int32Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765,
            32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648),
        Int32Array.from(Array.of<int>(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648)));
    checkNumberArray(Int32Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765,
            32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648),
        Int32Array.from([-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648]));
    checkNumberArray(Int32Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765,
            32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648), Int32Array.from(source2));
    checkNumberArray(Uint32Array.of(4294967295.0, 4294934530.0, 4294934529.0, 4294934528.0, 4294934527.0, 4294901761.0,
            4294901760.0, 4294901759.0, 0.0, 12.0, 255.0, 32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0,
            65536.0, 100000.0, 2147483647.0, 2147483648.0),
        Uint32Array.from(Array.of<int>(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648)));
    checkNumberArray(Uint32Array.of(4294967295.0, 4294934530.0, 4294934529.0, 4294934528.0, 4294934527.0, 4294901761.0,
            4294901760.0, 4294901759.0, 0.0, 12.0, 255.0, 32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0,
            65536.0, 100000.0, 2147483647.0, 2147483648.0),
        Uint32Array.from([-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12,
            255, 32766, 32765, 32768, 32769, 65534, 65535, 65536, 100000, 2147483647, -2147483648]));
    checkNumberArray(Uint32Array.of(4294967295.0, 4294934530.0, 4294934529.0, 4294934528.0, 4294934527.0, 4294901761.0,
            4294901760.0, 4294901759.0, 0.0, 12.0, 255.0, 32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0,
            65536.0, 100000.0, 2147483647.0, 2147483648.0), Uint32Array.from(source2));

    checkNumberArray(Int32Array.of(-1.0, -2147483647.0, -2147483648.0, 2147483647.0, 2.0, 1.0, 0.0, -1.0,
            0.0, 12.0, 255.0, 2147483646.0, 2147483647.0, -2147483648.0, -2147483647.0, -2, -1, 0.0),
        Int32Array.from(Uint32Array.of(-1.0, -2147483647.0, -2147483648.0, -2147483649.0, -4294967294.0, -4294967295.0,
            -4294967296.0, -4294967297.0, 0.0, 12.0, 255.0, 2147483646.0, 2147483647.0, 2147483648.0, 2147483649.0,
            4294967294.0, 4294967295.0, 4294967296.0)));
    checkNumberArray(Uint32Array.of(4294967295, 2147483649, 2147483648, 2147483647, 2, 1,
            0, 4294967295, 0, 12, 255, 2147483646, 2147483647, 2147483648, 2147483649,
            4294967294, 4294967295, 0.0),
        Uint32Array.from(Int32Array.from([-1.0, -2147483647.0, -2147483648.0, -2147483649.0, -4294967294.0, -4294967295.0,
            -4294967296.0, -4294967297.0, 0.0, 12.0, 255.0, 2147483646.0, 2147483647.0, 2147483648.0, 2147483649.0,
            4294967294.0, 4294967295.0, 4294967296.0])));
}

function testF32FromArrayWrongType() {
    let strings: Array<string> = Array.of(new string("hi"), new string("hello"), new string("new"));
    __noinline__Float32ArrayFromCheckException(strings, 'std.core.LineString');
    let ints: Object = Array.of(1, 2, 3);
    __noinline__Float32ArrayFromCheckException(ints, 'std.core.Int');
}

function testF32FromIterableNumber() {
    checkNumberArray(Float32Array.of(), Float32Array.from(new Set<number>()));

    const len: int = 127;
    let expected: Float32Array = new Float32Array(len);
    for (let i: int = 0; i < len; i++) {
        expected[i] = (i + 1.0) as number;
    }

    let data = prepareTestData(len, 1.0);
    let set = data[0];
    checkNumberArray(expected, Float32Array.from(set));
}

function testF32FromArrayLikeNumber() {
    let empty: FixedArray<number> = [];
    checkNumberArray(Float32Array.of(), Float32Array.from(new Array<number>()));
    checkNumberArray(Float32Array.of(), Float32Array.from([]));
    checkNumberArray(Float32Array.of(), Float32Array.from(empty));
    checkNumberArray(Float32Array.of(), Float32Array.from(Float64Array.of()));
    checkNumberArray(Float32Array.of(), Float32Array.from(Float32Array.of()));

    const len: int = 127;
    let expected: Float32Array = new Float32Array(len);
    let f64arr: Float64Array = new Float64Array(len);
    let f32arr: Float32Array = new Float32Array(len);
    for (let i: int = 0; i < len; i++) {
        expected[i] = f64arr[i] = f32arr[i] = (i + 1.0) as number;
    }

    let data = prepareTestData(len, 1.0);
    let arr = data[1];
    let fixarr = data[2];
    checkNumberArray(expected, Float32Array.from(arr));
    checkNumberArray(expected, Float32Array.from(fixarr));
    checkNumberArray(expected, Float32Array.from(f64arr));
    checkNumberArray(expected, Float32Array.from(f32arr));

    let source: FixedArray<number> = [1.0, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105,
        -3.31899e+38, 3.31899e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -3.402823466385289e+38, 3.402823466385289e+38,
        -3.4028235677973359e+38, 3.4028235677973359e+38, -3.4028235677973362e+38, 3.4028235677973362e+38,
        -3.4028235677973362e+38 - 1.0e+23, 3.4028235677973362e+38 + 1.0e+23];
    checkNumberArray(Float32Array.of(1, NaN, Infinity, 0.00001f, Infinity, -Infinity, -Infinity, -0.6e+16f, Infinity,
        -3.31899e+38, 3.31899e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -3.4028234663852886e+38, 3.4028234663852886e+38,
        -3.4028234663852886e+38, 3.4028234663852886e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -Infinity, Infinity),
        Float32Array.from(Array.of(1.0, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105,
            -3.31899e+38, 3.31899e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -3.402823466385289e+38, 3.402823466385289e+38,
            -3.4028235677973359e+38, 3.4028235677973359e+38, -3.4028235677973362e+38, 3.4028235677973362e+38,
            -3.4028235677973362e+38 - 1.0e+23, 3.4028235677973362e+38 + 1.0e+23)));
    checkNumberArray(Float32Array.of(1, NaN, Infinity, 0.00001f, Infinity, -Infinity, -Infinity, -0.6e+16f, Infinity,
        -3.31899e+38, 3.31899e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -3.4028234663852886e+38, 3.4028234663852886e+38,
        -3.4028234663852886e+38, 3.4028234663852886e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -Infinity, Infinity),
        Float32Array.from([1.0, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105,
            -3.31899e+38, 3.31899e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -3.402823466385289e+38, 3.402823466385289e+38,
            -3.4028235677973359e+38, 3.4028235677973359e+38, -3.4028235677973362e+38, 3.4028235677973362e+38,
            -3.4028235677973362e+38 - 1.0e+23, 3.4028235677973362e+38 + 1.0e+23]));
    checkNumberArray(Float32Array.of(1, NaN, Infinity, 0.00001f, Infinity, -Infinity, -Infinity, -0.6e+16f, Infinity,
        -3.31899e+38, 3.31899e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -3.4028234663852886e+38, 3.4028234663852886e+38,
        -3.4028234663852886e+38, 3.4028234663852886e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -Infinity, Infinity),
        Float32Array.from(source));

    let shortsource: FixedArray<number> = [1, 65535, -65535, NaN, Infinity];
    checkNumberArray(Float32Array.of(1, 65535, -65535, NaN, Infinity),
        Float32Array.from(Array.of<number>(1, 65535, -65535, NaN, Infinity)));
    checkNumberArray(Float32Array.of(1, 65535, -65535, NaN, Infinity),
        Float32Array.from([1, 65535, -65535, NaN, Infinity]));
    checkNumberArray(Float32Array.of(1, 65535, -65535, NaN, Infinity), Float32Array.from(shortsource));
}

function testF64FromIterableNumber() {
    checkNumberArray(Float64Array.of(), Float64Array.from(new Set<number>()));

    const len: int = 127;
    let expected: Float64Array = new Float64Array(len);
    for (let i = 0; i < len; i++) {
        expected[i] = (i + 1.0) as number;
    }

    let data = prepareTestData(len, 1.0);
    let set = data[0];
    checkNumberArray(expected, Float64Array.from(set));
}

function testF64FromArrayLikeNumber() {
    let empty: FixedArray<number> = [];
    checkNumberArray(Float64Array.of(), Float64Array.from(new Array<number>()));
    checkNumberArray(Float64Array.of(), Float64Array.from([]));
    checkNumberArray(Float64Array.of(), Float64Array.from(empty));
    checkNumberArray(Float64Array.of(), Float64Array.from(Float64Array.of()));
    checkNumberArray(Float64Array.of(), Float64Array.from(Float32Array.of()));

    const len: int = 127;
    let expected: Float64Array = new Float64Array(len);
    let f64arr: Float64Array = new Float64Array(len);
    let f32arr: Float32Array = new Float32Array(len);
    for (let i = 0; i < len; i++) {
        expected[i] = f64arr[i] = f32arr[i] = (i + 1.0) as number;
    }

    let data = prepareTestData(len, 1.0);
    let arr = data[1];
    let fixarr = data[2];
    checkNumberArray(expected, Float64Array.from(arr));
    checkNumberArray(expected, Float64Array.from(fixarr));
    checkNumberArray(expected, Float64Array.from(f64arr));
    checkNumberArray(expected, Float64Array.from(f32arr));

    let source: FixedArray<number> = [1.0, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105];
    checkNumberArray(Float64Array.of(1, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105),
        Float64Array.from(Array.of(1.0, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105)));
    checkNumberArray(Float64Array.of(1, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105),
        Float64Array.from([1.0, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105]));
    checkNumberArray(Float64Array.of(1, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105),
        Float64Array.from(source));
    let shortsource: FixedArray<number> = [1, 65535, -65535, NaN, Infinity];
    checkNumberArray(Float64Array.of(1, 65535, -65535, NaN, Infinity),
        Float64Array.from(Array.of<number>(1, 65535, -65535, NaN, Infinity)));
    checkNumberArray(Float64Array.of(1, 65535, -65535, NaN, Infinity),
        Float64Array.from([1, 65535, -65535, NaN, Infinity]));
    checkNumberArray(Float64Array.of(1, 65535, -65535, NaN, Infinity), Float64Array.from(shortsource));
}

function testIU64FromArrayLikeNumber() {
    checkException(() => { return BigInt64Array.from(Int16Array.of()) }, BigInt64Array.of());
    checkException(() => { return BigInt64Array.from(new Array<number>()) }, BigInt64Array.of());
    checkException(() => { return BigInt64Array.from(Int8Array.of()) }, BigInt64Array.of());
    checkException(() => { return BigInt64Array.from(Uint8Array.of()) }, BigInt64Array.of());
    checkException(() => { return BigUint64Array.from(BigInt64Array.of()) }, BigUint64Array.of());
}

function checkException<Arr>(creator: () => Arr, empty: Arr): void {
    arktest.expectThrow(() => {creator();}, (e: Error) => {
        const arrayClassName: string = (Type.of(empty) as ClassType).getName().substring('escompat.'.length);
        return e.message == arrayClassName + '.from: not implemented';
    });
}

function __noinline__Int8ArrayFromCheckException(obj: Object, factType: string): void
{
    let arr: Array<int> = obj as Array<int>;
    arktest.expectThrow(() => { Int8Array.from(arr); }, (e: Error) => {
        return e instanceof ClassCastError && e.message == factType + ' cannot be cast to std.core.Int';
    });
}

function __noinline__Float32ArrayFromCheckException(obj: Object, factType: string): void
{
    let arr: Array<number> = obj as Array<number>;
    arktest.expectThrow(() => { Float32Array.from(arr); }, (e: Error) => {
        return e instanceof ClassCastError && e.message == factType + ' cannot be cast to std.core.Double';
    });
}

function __noinline__BigInt64ArrayFromArrayCheckException(obj: Object, factType: string): void
{
    let arr: Array<bigint> = obj as Array<bigint>;
    arktest.expectThrow(() => { BigInt64Array.from(arr); }, (e: Error) => {
        return e instanceof ClassCastError && e.message == factType + ' cannot be cast to std.core.BigInt';
    });
}

function __noinline__BigInt64ArrayFromFixedArrayCheckException(obj: Object, factType: string): void
{
    arktest.expectThrow(() => { let arr: FixedArray<bigint> = obj as FixedArray<bigint> }, (e: Error) => {
        return e instanceof ClassCastError && e.message == factType + ' cannot be cast to [Lstd/core/BigInt;';
    });
}

function testIU64FromArrayWrongType() {
    let strings: Array<string> = Array.of(new string("hi"), new string("hello"), new string("new"));
    __noinline__BigInt64ArrayFromArrayCheckException(strings, 'std.core.LineString');
    let ints: Object = Array.of(1, 2, 3);
    __noinline__BigInt64ArrayFromArrayCheckException(ints, 'std.core.Int');
    let fixstrings: FixedArray<string> = [new string("hi"), new string("hello"), new string("new")];
    __noinline__BigInt64ArrayFromFixedArrayCheckException(fixstrings, '[Lstd/core/String;');
}

function testIU64FromArrayBigInt() {
    let empty: FixedArray<bigint> = [];
    checkBigIntArray(BigInt64Array.of(), BigInt64Array.from(new Array<bigint>()));
    checkBigIntArray(BigInt64Array.of(), BigInt64Array.from([]));
    checkBigIntArray(BigInt64Array.of(), BigInt64Array.from(empty));

    const length: int = 128;
    let arr: Array<bigint> = new Array<bigint>(length);
    let fixarr: FixedArray<bigint> = new FixedArray<bigint>(length);
    let iexpected: BigInt64Array = new BigInt64Array(length);
    for (let i = 0; i < length; i++) {
        const v = new BigInt(i);
        arr[i] = v;
        fixarr[i] = v;
        iexpected[i] = v;
    }
    checkBigIntArray(iexpected, BigInt64Array.from(arr));
    checkBigIntArray(iexpected, BigInt64Array.from(fixarr));
    let arr2: Array<bigint> = Array.of(1n, BigInt(1.e+5), BigInt(1.7e+308), BigInt(-1.7e+308), BigInt(-0.6e+16),
        BigInt(0.4e+105), 9007199254740991n * 9007199254740991n, -9007199254740991n * 9007199254740991n,
        -9007199254740991n * 9007199254740991n * 9007199254740991n * 9007199254740991n * 9007199254740991n);

    let fixarr2: FixedArray<bigint> = [1n, BigInt(1.e+5), BigInt(1.7e+308), BigInt(-1.7e+308), BigInt(-0.6e+16),
        BigInt(0.4e+105), 9007199254740991n * 9007199254740991n, -9007199254740991n * 9007199254740991n,
        -9007199254740991n * 9007199254740991n * 9007199254740991n * 9007199254740991n * 9007199254740991n];
    checkBigIntArray(BigInt64Array.of(1n, BigInt(1.e+5), 0n, 0n, BigInt(-0.6e+16), 0n, -18014398509481983n,
            18014398509481983n, -45035996273704959n),
        BigInt64Array.from(arr2));
    checkBigIntArray(BigInt64Array.of(1n, BigInt(1.e+5), 0n, 0n, BigInt(-0.6e+16), 0n, -18014398509481983n,
            18014398509481983n, -45035996273704959n),
        BigInt64Array.from(fixarr2));
    // Currently, we have no method to build a BigUint64Array from an Array<bigint>
    let uexpected: BigUint64Array = new BigUint64Array(arr2.length);
    for (let i = 0; i < arr2.length; i++) {
        uexpected[i] = arr2[i];
    }
    checkBigIntArray(BigUint64Array.of(1n, 100000n, 0n, 0n, 18440744073709551616n, 0n, 18428729675200069633n,
        18014398509481983n, 18401708077435846657n), uexpected);
}

function testIU64FromIterableBigInt() {
    checkBigIntArray(BigInt64Array.of(), BigInt64Array.from(new Set<bigint>()));
    checkBigIntArray(BigUint64Array.of(), BigUint64Array.from(new Set<bigint>()));
    checkBigIntArray(BigInt64Array.of(), BigInt64Array.from(BigInt64Array.of()));
    checkBigIntArray(BigInt64Array.of(), BigInt64Array.from(BigUint64Array.of()));

    const length: int = 128;
    let set: Set<bigint> = new Set<bigint>();
    let i64Arr: BigInt64Array = new BigInt64Array(length);
    let u64Arr: BigUint64Array = new BigUint64Array(length);
    let iexpected: BigInt64Array = new BigInt64Array(length);
    let uexpected: BigUint64Array = new BigUint64Array(length);

    for (let i = 0; i < length; i++) {
        const v = new BigInt(i);
        set.add(v);
        i64Arr[i] = v;
        u64Arr[i] = v;
        iexpected[i] = v;
        uexpected[i] = v;
    }
    checkBigIntArray(iexpected, BigInt64Array.from(set));
    checkBigIntArray(uexpected, BigUint64Array.from(set));
    checkBigIntArray(iexpected, BigInt64Array.from(i64Arr));
    checkBigIntArray(iexpected, BigInt64Array.from(u64Arr));
}

function prepareTestData(length: int, from: int): [Set<int>, Array<int>, FixedArray<int>] {
    let set: Set<int> = new Set<int>();
    let arr: Array<int> = new Array<int>(length);
    let fixarr: FixedArray<int> = new FixedArray<int>(length);
    let v: int = from;
    for (let i = 0; i < length; i++) {
        arr[i] = v;
        fixarr[i] = v;
        set.add(v);
        v++;
    }
    return [set, arr, fixarr];
}

function prepareTestData(length: int, from: number): [Set<number>, Array<number>, FixedArray<number>] {
    let set: Set<number> = new Set<number>();
    let arr: Array<number> = new Array<number>(length);
    let fixarr: FixedArray<number> = new FixedArray<number>(length);
    let v: number = from;
    for (let i = 0; i < length; i++) {
        arr[i] = v;
        fixarr[i] = v;
        set.add(v);
        v++;
    }
    return [set, arr, fixarr];
}

function checkNumberArray(expected: ArrayLike<number>, data: ArrayLike<number>): void {
    arktest.assertEQ(data.length, expected.length, 'Unexpected array length');
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertTrue((isNaN(data[i]) && isNaN(expected[i])) || (data[i] == expected[i]),
            'Unexpected element with index ' + i + ': expected ' + expected[i] + ' but was ' + data[i]);
    }
}

function checkBigIntArray(expected: ArrayLike<BigInt>, data: ArrayLike<BigInt>): void {
    arktest.assertEQ(data.length, expected.length, 'Unexpected array length');
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertEQ(data[i], expected[i], 'Unexpected element with index ' + i);
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('typedArrays.from');
    testSuite.addTest('BigInt64,BigUint64 Arrays: from Array of a wrong type test', testIU64FromArrayWrongType);
    testSuite.addTest('BigInt64,BigUint64 Arrays: from Iterable<BigInt> test', testIU64FromIterableBigInt);
    testSuite.addTest('BigInt64,BigUint64 Arrays: from Array<BigInt> test', testIU64FromArrayBigInt);
    testSuite.addTest('BigInt64,BigUint64 Arrays: from ArrayLike<number> test', testIU64FromArrayLikeNumber);
    testSuite.addTest('Int8,Uint8 Arrays: from Array of a wrong type test', testIU8FromArrayWrongType);
    testSuite.addTest('Int8,Uint8 Arrays: from Iterable<number> test', testIU8FromIterableNumber);
    testSuite.addTest('Int8,Uint8 Arrays: from ArrayLike<number> test', testIU8FromArrayLikeNumber);
    testSuite.addTest('Int16,Uint16 Arrays: from Iterable<number> test', testIU16FromIterableNumber);
    testSuite.addTest('Int16,Uint16 Arrays: from ArrayLike<number> test', testIU16FromArrayLikeNumber);
    testSuite.addTest('Int32,Uint32 Arrays: from Iterable<number> test', testIU32FromIterableNumber);
    testSuite.addTest('Int32,Uint32 Arrays: from ArrayLike<number> test', testIU32FromArrayLikeNumber);
    testSuite.addTest('Float32 Arrays: from Array of a wrong type test', testF32FromArrayWrongType);
    testSuite.addTest('Float32 Arrays: from Iterable<number> test', testF32FromIterableNumber);
    testSuite.addTest('Float32 Arrays: from ArrayLike<number> test', testF32FromArrayLikeNumber);
    testSuite.addTest('Float64 Arrays: from Iterable<number> test', testF64FromIterableNumber);
    testSuite.addTest('Float64 Arrays: from ArrayLike<number> test', testF64FromArrayLikeNumber);
    return testSuite.run();
}
