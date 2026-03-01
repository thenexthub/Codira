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

import buffer from "@ohos.buffer"


function main(): int {
    const suite = new arktest.ArkTestsuite("Buffer Blob API tests")

    suite.addAsyncTest("Test get Blob slice from empty String array", testEmptyStringSlice)
    suite.addAsyncTest("Test get Blob slice from Chinese String array without options", testChineseStringSlice)
    suite.addAsyncTest("Test get Blob slice from Chinese and English String array without options", testEnglishStringSlice)
    suite.addAsyncTest("Test create Blob from String array without options", testBlobFromStringArray)
    suite.addAsyncTest("Test create Blob from ArrayBuffer without options", testBlobFromArrayBuffer)
    suite.addAsyncTest("Test create Blob from TypedArray without options", testBlobFromTypedArray)
    suite.addAsyncTest("Test create Blob from DataView without options", testBlobFromDataView)
    suite.addAsyncTest("Test create Blob from other Blob without options", testBlobFromBlob)
    suite.addAsyncTest("Test create Blob from String array with options", testBlobWithOptions)
    suite.addAsyncTest("Test get Blob slice from String array without options", testStringArraySlice)
    suite.addAsyncTest("Test get Blob slice from ArrayBuffer without options", testArrayBufferSlice)
    suite.addAsyncTest("Test get Blob slice from TypedArray without options", testTypedArraySlice)
    suite.addAsyncTest("Test get Blob slice from DataView without options", testDataViewSlice)
    suite.addAsyncTest("Test get Blob slice from other Blob without options", testBlobSlice)
    suite.addAsyncTest("Test get Blob toArrayBuffer", testBlobToArrayBuffer)
    suite.addAsyncTest("Test get Blob constructor ArrayString", testBlobToArrayString)
    suite.addAsyncTest("Test get Blob constructor ArrayBuffer", testBlobToArrayArrayBuffer)
    suite.addAsyncTest("Test get Blob constructor Int8Array", testBlobToArrayInt8Array)
    suite.addAsyncTest("Test get Blob constructor Uint8Array", testBlobToArrayUint8Array)
    suite.addAsyncTest("Test get Blob constructor Uint8ClampedArray", testBlobToArrayUint8ClampedArray)
    suite.addTest("Test get Blob constructor Int16Array", testBlobToArrayInt16Array)
    suite.addTest("Test get Blob constructor Uint16Array", testBlobToArrayUint16Array)
    suite.addAsyncTest("Test get Blob constructor Uint8Array with options", testBlobToArrayUint8ArrayOptions)
    suite.addTest("Test get Blob constructor Int32Array with options", testBlobToInt32Array)
    suite.addTest("Test get Blob constructor Uint32Array with options", testBlobToUint32Array)
    suite.addTest("Test get Blob constructor Float32Array with options", testBlobToFloat32Array)
    suite.addTest("Test get Blob constructor Float64Array with options", testBlobToFloat64Array)
    suite.addTest("Test get Blob constructor BigInt64Array with options", testBlobToBigInt64Array)
    suite.addTest("Test get Blob constructor BigUint64Array with options", testBlobToBigUint64Array)
    suite.addAsyncTest("Test get Blob constructor for string[]", testBlobToBuiltInStringArray)
    suite.addAsyncTest("Test get Blob constructor from empty", testBlobFromEmpty)
    suite.addTest("Test get Blob constructor from arrays with negative values", testBlobNegative)

    return suite.run()
}

async function testEmptyStringSlice() {
    let sourcesFixed: FixedArray<string>= [];
    let blobFixed = new buffer.Blob(sourcesFixed)
    let sizeFixed = blobFixed.size;
    arktest.assertEQ(blobFixed.type, '')
    arktest.assertEQ(sizeFixed, 0)
    let textFixed = await blobFixed.text()
    arktest.assertEQ(textFixed, '')
    let valueFixed = await blobFixed.arrayBuffer()
    arktest.assertEQ(valueFixed.byteLength, 0)

    let sources = [''];
    let blob = new buffer.Blob(sources)
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 0)
    let text = await blob.text()
    arktest.assertEQ(text, '')
    let value = await blobFixed.arrayBuffer()
    arktest.assertEQ(value.byteLength, 0)
}

async function testChineseStringSlice() {
    let sources: FixedArray<string> = ['你好', '世界', '测试'];
    let blob = new buffer.Blob(sources);
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 18)
    let text = await blob.text()
    arktest.assertEQ(text, '你好世界测试')
}

async function testEnglishStringSlice() {
    let sources: FixedArray<string> = ['测试', '$#', 'c'];
    let blob = new buffer.Blob(sources)
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 9)
    let text = await blob.text()
    arktest.assertEQ(text, '测试$#c')
}

async function testBlobFromStringArray() {
    let src: FixedArray<string> = ['1', '2', '3', '4', '5']
    let blob = new buffer.Blob(src)
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(blob.size, 5)
    let text = await blob.text()
    arktest.assertEQ(text, '12345')
}

async function testBlobFromArrayBuffer() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    let src: FixedArray<ArrayBuffer> = [arrayBuffer]
    let blob = new buffer.Blob(src)
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(blob.size, 5)
    let text = await blob.text()
    arktest.assertEQ(text, 'Hello')
}

async function testBlobFromTypedArray() {
    let src: FixedArray<Int8Array> = [new Int8Array([97, 98, 99, 97, 98])]
    let blob = new buffer.Blob(src)
    arktest.assertEQ(blob.size, 5)
    arktest.assertEQ(blob.type, '')
    let text = await blob.text()
    arktest.assertEQ(text, 'abcab')
}

async function testBlobFromDataView() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    const dataView = new DataView(arrayBuffer)
    let src: FixedArray<DataView> = [dataView, dataView, dataView]
    let blob = new buffer.Blob(src)
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(blob.size, 15)
    let text = await blob.text()
    arktest.assertEQ(text, 'HelloHelloHello')
}

async function testBlobFromBlob() {
    let src = new buffer.Blob(['1', '2', '3', '4', '5'])
    let blob = new buffer.Blob([src] as FixedArray<buffer.Blob>)
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(blob.size, 5)
    let text = await blob.text()
    arktest.assertEQ(text, '12345')
}

async function testBlobWithOptions() {
    const blobParts: FixedArray<string> = ['<q id="a"><span id="b">hey!</span></q>'];
    const blob = new buffer.Blob(blobParts, { type: 'text/html', endings: 'transparent' } as buffer.BlobOptions);
    arktest.assertEQ(blob.type, 'text/html')
    let text = await blob.text()
    arktest.assertEQ(text, '<q id="a"><span id="b">hey!</span></q>')
}

async function testStringArraySlice() {
    let src: FixedArray<string> = ['1', '2', '3', '4', '5']
    let blob = new buffer.Blob(src).slice(0,3)
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(blob.size, 3)
    let text = await blob.text()
    arktest.assertEQ(text, '123')
}

async function testArrayBufferSlice() {
    let bufferArr = new ArrayBuffer(3);
    let uint8arr = new Uint8Array(bufferArr);
    uint8arr[0] = 97;
    uint8arr[1] = 98;
    uint8arr[2] = 99;
    let bufferArrSrc = new ArrayBuffer(4);
    let uint8Arr = new Uint8Array(bufferArrSrc);
    uint8Arr[0] = 100;
    uint8Arr[1] = 101;
    uint8Arr[2] = 102;
    uint8Arr[3] = 103;
    let blobArr = new buffer.Blob([bufferArr, bufferArrSrc]).slice(1, 5);
    let sizeArr = blobArr.size;
    arktest.assertEQ(blobArr.type, '');
    arktest.assertEQ(sizeArr, 4);
    let textArr = await blobArr.text();
    arktest.assertEQ(textArr, 'bcde');
    let value = await blobArr.arrayBuffer()
    let uint8Array = new Uint8Array(value);
    arktest.assertEQ(uint8Array[0], 98);
    arktest.assertEQ(uint8Array[1], 99);
    arktest.assertEQ(uint8Array[2], 100);
    arktest.assertEQ(uint8Array[3], 101);

    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    let src: FixedArray<ArrayBuffer> = [arrayBuffer, arrayBuffer, arrayBuffer, arrayBuffer]
    let blob = new buffer.Blob(src).slice(1,2)
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(blob.size, 1)
    let text = await blob.text()
    arktest.assertEQ(text, 'e')
}

async function testTypedArraySlice() {
    let src: FixedArray<Int8Array> = [new Int8Array([97, 98, 99]),
                            new Int8Array([100, 101]),
                            new Int8Array([102, 103]),
                            new Int8Array([97, 98, 100]),
                            new Int8Array([110, 120, 130, 140])]
    let blob = new buffer.Blob(src).slice(0, 1)
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(blob.size, 1)
    let text = await blob.text()
    arktest.assertEQ(text, 'a')
}

async function testDataViewSlice() {
    const arr = new Uint8Array([72, 101, 108, 108, 111]);
    const arr1 = new Uint8Array([97, 98, 99, 100]);
    const arrayBuffer = arr.buffer as ArrayBuffer;
    const arrayBuffer1 = arr1.buffer as ArrayBuffer;
    let src: FixedArray<DataView> = [new DataView(arrayBuffer), new DataView(arrayBuffer1)];
    let blob = new buffer.Blob(src).slice();
    arktest.assertEQ(blob.type, '');
    arktest.assertEQ(blob.size, 9);
    let text = await blob.text();
    arktest.assertEQ(text, 'Helloabcd');
    let value = await blob.arrayBuffer();
    let arrayValue = new Uint8Array(value);
    arktest.assertEQ(arrayValue[8], 100);

    let bufferArr = new ArrayBuffer(5);
    let uint8arr = new Uint8Array(bufferArr);
    uint8arr[0] = 72;
    uint8arr[1] = 101;
    uint8arr[2] = 108;
    uint8arr[3] = 108;
    uint8arr[4] = 111;
    let bufferArrSrc = new ArrayBuffer(4);
    let uint8Arr = new Uint8Array(bufferArrSrc);
    uint8Arr[0] = 97;
    uint8Arr[1] = 98;
    uint8Arr[2] = 99;
    uint8Arr[3] = 100;
    let srcDataView = new DataView(bufferArr);
    let srcDataView1 = new DataView(bufferArrSrc);
    let blobData = new buffer.Blob([srcDataView, srcDataView1]).slice(0, 5, 'MIME');
    arktest.assertEQ(blobData.type, 'MIME');
    arktest.assertEQ(blobData.size, 5);
    let textData = await blobData.text();
    arktest.assertEQ(textData, 'Hello');
    let valueData = await blobData.arrayBuffer();
    let arrayData = new Uint8Array(valueData);
    arktest.assertEQ(arrayData[2], 108);
}

async function testBlobSlice() {
    let src1 = new buffer.Blob(['1', '2', '3', '4', '5'])
    let src2 = new buffer.Blob([new Int8Array([1, 2, 3, 4, 5])] as FixedArray<Int8Array>)
    let src3 = new buffer.Blob(['1', '2', '3', '4', '5'])
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    let src4 = new buffer.Blob([arrayBuffer] as FixedArray<ArrayBuffer>)
    let blob = new buffer.Blob([src1, src2, src3, src4] as FixedArray<buffer.Blob>).slice(0, 5)
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(blob.size, 5)
    let text = await blob.text()
    arktest.assertEQ(text, '12345')
    let value = await blob.arrayBuffer();
    let arrayValue = new Uint8Array(value)
    let len = arrayValue.length;
    for (let i = 0; i < len; i++) {
        arktest.assertEQ(arrayValue[i], 49 + i);
    }

    let srcArray1 = new buffer.Blob(['A', 'B', 'C', 'D', 'E'])
    let srcArray2 = new buffer.Blob([new Int8Array([72, 101, 108, 108, 111])])
    let srcArray3 = new buffer.Blob(['ab', 'cd', 'ef'])
    let bufferArr = new ArrayBuffer(5);
    let uint8arr = new Uint8Array(bufferArr);
    uint8arr[0] = 65;
    uint8arr[1] = 66;
    uint8arr[2] = 67;
    uint8arr[3] = 68;
    uint8arr[4] = 69;
    let srcArray4 = new buffer.Blob([bufferArr])
    let blobArray = new buffer.Blob([srcArray1, srcArray2, srcArray3, srcArray4]).slice(1, 5)
    arktest.assertEQ(blobArray.type, '')
    arktest.assertEQ(blobArray.size, 4)
    let textArray = await blobArray.text()
    arktest.assertEQ(textArray, 'BCDE')
    let valueArray = await blobArray.arrayBuffer();
    let array = new Uint8Array(valueArray)
    let lenArray = array.length;
    for (let i = 0; i < lenArray; i++) {
        arktest.assertEQ(array[i], 66 + i);
    }
}

async function testBlobToArrayBuffer() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    let src: FixedArray<DataView> = [new DataView(arrayBuffer)]
    let blob = new buffer.Blob(src)
    let arrayBufferFromBlob = await blob.arrayBuffer()
    let text = await blob.text()
    arktest.assertEQ(text, 'Hello')
    arktest.assertEQ(ArrayBuffer.stringify(arrayBufferFromBlob, 'utf8', 0, arrayBufferFromBlob.getByteLength()), text)
}

async function testBlobToArrayString() {
    let sources = ['测试', '$#', 'c'];
    let blob = new buffer.Blob(sources)
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 9)
    let text = await blob.text()
    arktest.assertEQ(text, '测试$#c')
}

async function testBlobToArrayArrayBuffer() {
    let arr = new ArrayBuffer(3);
    let uint8arr = new Uint8Array(arr);
    uint8arr[0] = 97;
    uint8arr[1] = 98;
    uint8arr[2] = 99;
    let blob = new buffer.Blob([arr]);
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 3)
    let text = await blob.text()
    arktest.assertEQ(text, 'abc')
}

async function testBlobToArrayInt8Array() {
    {
        let src: FixedArray<Int8Array> = [new Int8Array([1, 2, 3]),
                            new Int8Array([4, 5]),
                            new Int8Array([6, 7]),
                            new Int8Array([8, 9, 10]),
                            new Int8Array([11, 12, 13, 14])]
        let blob = new buffer.Blob(src).slice(2, 4);
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(blob.size, 2)
        let text = await blob.text()
        arktest.assertEQ(text, '\u0003\u0004')
        let valueArray = await blob.arrayBuffer();
        let array = new Uint8Array(valueArray)
        arktest.assertEQ(array[0], 3)
        arktest.assertEQ(array[1], 4)
    }
    {
        let src: Int8Array[] = [new Int8Array([1, 2, 3]),
                            new Int8Array([4, 5]),
                            new Int8Array([6, 7]),
                            new Int8Array([8, 9, 10]),
                            new Int8Array([11, 12, 13, 14])]
        let blob = new buffer.Blob(src).slice(2, 4);
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(blob.size, 2)
        let text = await blob.text()
        arktest.assertEQ(text, '\u0003\u0004')
        let valueArray = await blob.arrayBuffer();
        let array = new Uint8Array(valueArray)
        arktest.assertEQ(array[0], 3)
        arktest.assertEQ(array[1], 4)
    }
    {
        let src: Int8Array[] = [new Int8Array([1, 2, 3, 4, 5])]
        let blob = new buffer.Blob(src);
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(blob.size, 5)
        let text = await blob.text()
        arktest.assertEQ(text, '\u0001\u0002\u0003\u0004\u0005')
        let valueArray = await blob.arrayBuffer();
        let array = new Uint8Array(valueArray)
        arktest.assertEQ(array[0], 1)
        arktest.assertEQ(array[1], 2)
        arktest.assertEQ(array[2], 3)
        arktest.assertEQ(array[3], 4)
        arktest.assertEQ(array[4], 5)
    }
}

async function testBlobToArrayUint8Array() {
    let uint8arr1 = new Uint8Array(3);
    uint8arr1[0] = 97;
    uint8arr1[1] = 98;
    uint8arr1[2] = 99;
    let blob = new buffer.Blob([uint8arr1]);
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 3)
    let text = await blob.text()
    arktest.assertEQ(text, 'abc')
}

async function testBlobToArrayUint8ClampedArray() {
    let uintc8 = new Uint8ClampedArray(3);
    uintc8 [0] = 100;
    uintc8 [1] = 101;
    uintc8 [2] = 102;
    let blob = new buffer.Blob([uintc8]);
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 3)
    let text = await blob.text()
    arktest.assertEQ(text, 'def')
}

function testBlobToArrayInt16Array() {
    let int16 = new Int16Array(2);
    int16 [0] = 100;
    int16 [1] = 101;
    let blob = new buffer.Blob([int16]);
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 4)
}

function testBlobToArrayUint16Array() {
    let uint16 = new Uint16Array(5);
    uint16[0] = 72;
    uint16[1] = 101;
    uint16[2] = 108;
    uint16[3] = 108;
    uint16[4] = 111;
    let blob = new buffer.Blob([uint16]);
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 10)
}

async function testBlobToArrayUint8ArrayOptions() {
    let o1: buffer.BlobOptions = { type: 'new type', endings: 'transparent' };
    let uint8arr1 = new Uint8Array(3);
    uint8arr1[0] = 97;
    uint8arr1[1] = 98;
    uint8arr1[2] = 99;
    let blob = new buffer.Blob([uint8arr1], o1);
    let size = blob.size;
    arktest.assertEQ(blob.type, 'new type')
    arktest.assertEQ(size, 3)
    let text = await blob.text()
    arktest.assertEQ(text, 'abc')
}

function testBlobToInt32Array() {
    let int32array = new Int32Array([97, 98, 99, 100]);
    let int32arraySrc = new Int32Array([97, 98, 99, 100]);
    let blob = new buffer.Blob([int32array, int32arraySrc]).slice(1, 6);
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 5)

    let FixedArraySrc: FixedArray<Int32Array> = [new Int32Array([97, 98, 99, 100]), new Int32Array([97, 98, 99, 100])];
    let blobFixed = new buffer.Blob(FixedArraySrc).slice(1, 6);
    let sizeFixed = blobFixed.size;
    arktest.assertEQ(blobFixed.type, '')
    arktest.assertEQ(sizeFixed, 5)
}

function testBlobToUint32Array() {
    let o1: buffer.BlobOptions = { type: 'new type', endings: 'transparent' };
    let uint32array = new Uint32Array([110, 120, 130, 140]);
    let uint32arraySrc = new Uint32Array([97, 98, 99, 100]);
    let blob = new buffer.Blob([uint32array, uint32arraySrc], o1).slice(2, 5, 'MIME');
    let size = blob.size;
    arktest.assertEQ(blob.type, 'MIME')
    arktest.assertEQ(size, 3)

    let FixedArraySrc: FixedArray<Uint32Array> = [new Uint32Array([110, 120, 130, 140]), new Uint32Array([97, 98, 99, 100])];
    let blobFixed = new buffer.Blob(FixedArraySrc, o1).slice(2, 5, 'MIME');
    let sizeFixed = blobFixed.size;
    arktest.assertEQ(blobFixed.type, 'MIME')
    arktest.assertEQ(sizeFixed, 3)
}

function testBlobToFloat32Array() {
    let o1: buffer.BlobOptions = { type: 'new type', endings: 'transparent' };
    let float32Array = new Float32Array([110, 120, 130, 140]);
    let float32ArraySrc = new Float32Array([97, 98, 99, 100]);
    let blob = new buffer.Blob([float32Array, float32ArraySrc], o1).slice(5, 6);
    let size = blob.size;
    arktest.assertEQ(blob.type, '')
    arktest.assertEQ(size, 1)

    let FixedArraySrc: FixedArray<Float32Array> = [new Float32Array([110, 120, 130, 140]), new Float32Array([97, 98, 99, 100])];
    let blobFixed = new buffer.Blob(FixedArraySrc, o1).slice(2, 5);
    let sizeFixed = blobFixed.size;
    arktest.assertEQ(blobFixed.type, '')
    arktest.assertEQ(sizeFixed, 3)
}

function testBlobToFloat64Array() {
    let o1: buffer.BlobOptions = { type: 'new type', endings: 'transparent' };
    let float64Array = new Float64Array([97, 98]);
    let float64ArraySrc = new Float64Array([99, 100, 101]);
    let blob = new buffer.Blob([float64Array, float64ArraySrc], o1)
    let size = blob.size;
    arktest.assertEQ(blob.type, 'new type')
    arktest.assertEQ(size, 40)

    let FixedArraySrc: FixedArray<Float64Array> = [new Float64Array([97, 98]), new Float64Array([99, 100, 101])];
    let blobFixed = new buffer.Blob(FixedArraySrc, o1);
    let sizeFixed = blobFixed.size;
    arktest.assertEQ(blobFixed.type, 'new type')
    arktest.assertEQ(sizeFixed, 40)
}

function testBlobToBigInt64Array() {
   let o1: buffer.BlobOptions = { type: 'new type', endings: 'transparent' };
    let bigInt64Array = new BigInt64Array([65n, 66n]);
    let bigInt64ArraySrc = new BigInt64Array([67n, 68n, 69n, 70n]);
    let blob = new buffer.Blob([bigInt64Array, bigInt64ArraySrc], o1).slice(0, 48, 'MIME')
    let size = blob.size;
    arktest.assertEQ(blob.type, 'MIME')
    arktest.assertEQ(size, 48)

    let FixedArraySrc: FixedArray<BigInt64Array> = [new BigInt64Array([65n, 66n]), new BigInt64Array([67n, 68n, 69n, 70n])];
    let blobFixed = new buffer.Blob(FixedArraySrc, o1).slice(0, 48, 'MIME');
    let sizeFixed = blobFixed.size;
    arktest.assertEQ(blobFixed.type, 'MIME')
    arktest.assertEQ(sizeFixed, 48)
}

function testBlobToBigUint64Array() {
   let o1: buffer.BlobOptions = { type: 'new type', endings: 'transparent' };
    let bigUint64Array = new BigUint64Array([97n, 97n, 99n, 100n]);
    let bigUint64ArraySrc = new BigUint64Array([67n, 68n, 69n, 70n]);
    let blob = new buffer.Blob([bigUint64Array, bigUint64ArraySrc], o1).slice(1, 20, 'MIME')
    let size = blob.size;
    arktest.assertEQ(blob.type, 'MIME')
    arktest.assertEQ(size, 19)

    let FixedArraySrc: FixedArray<BigUint64Array> = [new BigUint64Array([65n, 66n]), new BigUint64Array([67n, 68n, 69n, 70n])];
    let blobFixed = new buffer.Blob(FixedArraySrc, o1).slice(1, 20, 'MIME')
    let sizeFixed = blobFixed.size;
    arktest.assertEQ(blobFixed.type, 'MIME')
    arktest.assertEQ(sizeFixed, 19)
}



async function testBlobToBuiltInStringArray() {
    let src: string[] = ['1', '2', '3', '4', '5']
    let blob = new buffer.Blob(src)
    let text = await blob.text()
    arktest.assertEQ(text, '12345')
}

async function testBlobFromEmpty() {
    {
        let src: string[] = []
        let blob = new buffer.Blob(src)
        let text = await blob.text()
        arktest.assertEQ(text, '')
        arktest.assertEQ(blob.size, 0)
        arktest.assertEQ(blob.type, '')
    }
    {
        let src = new Uint8Array(0);
        let blob = new buffer.Blob([src])
        let text = await blob.text()
        arktest.assertEQ(text, '')
        arktest.assertEQ(blob.size, 0)
        arktest.assertEQ(blob.type, '')
    }
    {
        let src = new Uint16Array(0)
        let blob = new buffer.Blob([src])
        let text = await blob.text()
        arktest.assertEQ(text, '')
        arktest.assertEQ(blob.size, 0)
        arktest.assertEQ(blob.type, '')
    }
    {
        let src = new Int16Array(0)
        let blob = new buffer.Blob([src])
        let text = await blob.text()
        arktest.assertEQ(text, '')
        arktest.assertEQ(blob.size, 0)
        arktest.assertEQ(blob.type, '')
    }
    {
        let src = new Int8Array(0)
        let blob = new buffer.Blob([src])
        let text = await blob.text()
        arktest.assertEQ(text, '')
        arktest.assertEQ(blob.size, 0)
        arktest.assertEQ(blob.type, '')
    }
    {
        let src = new Uint8ClampedArray(0)
        let blob = new buffer.Blob([src])
        let text = await blob.text()
        arktest.assertEQ(text, '')
        arktest.assertEQ(blob.size, 0)
        arktest.assertEQ(blob.type, '')
    }
    {
        let src = new ArrayBuffer(0)
        let blob = new buffer.Blob([src])
        let text = await blob.text()
        arktest.assertEQ(text, '')
        arktest.assertEQ(blob.size, 0)
        arktest.assertEQ(blob.type, '')
    }

    {
        let src = new DataView(new ArrayBuffer(0));
        let blob = new buffer.Blob([src])
        let text = await blob.text()
        arktest.assertEQ(text, '')
        arktest.assertEQ(blob.size, 0)
        arktest.assertEQ(blob.type, '')
    }
}

function testBlobNegative() {
    {
        let int16 = new Int16Array(2);
        int16 [0] = -100;
        int16 [1] = -101;
        let blob = new buffer.Blob([int16]);
        let size = blob.size;
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(size, 4)
    }
    {
        let src = new Int8Array(2);
        src[0] = -100;
        src[1] = -101;
        let blob = new buffer.Blob([src]);
        let size = blob.size;
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(size, 2)
    }
    {
        let src = new Int32Array(2);
        src[0] = -100;
        src[1] = -101;
        let blob = new buffer.Blob([src]);
        let size = blob.size;
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(size, 8)
    }
    {
        let src = new Float32Array(2);
        src[0] = -100;
        src[1] = -101;
        let blob = new buffer.Blob([src]);
        let size = blob.size;
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(size, 8)
    }
    {
        let src = new Float64Array(2);
        src[0] = -100;
        src[1] = -101;
        let blob = new buffer.Blob([src]);
        let size = blob.size;
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(size, 16)
    }
    {
        let src = new BigInt64Array(2);
        src[0] = -100;
        src[1] = -101;
        let blob = new buffer.Blob([src]);
        let size = blob.size;
        arktest.assertEQ(blob.type, '')
        arktest.assertEQ(size, 16)
    }
}
