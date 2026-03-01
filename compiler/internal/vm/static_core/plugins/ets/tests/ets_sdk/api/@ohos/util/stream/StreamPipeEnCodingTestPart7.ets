/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { BusinessError } from "@ohos.base";
import { stream } from "@ohos.util.stream";

const mustChecks: Array<Function> = new Array<Function>();
function main(): int {
    const suite = new arktest.ArkTestsuite("Stream API tests");

    suite.addAsyncTest("Transform stream encoding validation (gb2312).", testTransformStreamWriteEncodingGb2312);
    suite.addTest("Readable stream flowing state validation.", testReadableStreamReadableFlowingState);
    suite.addTest("Readable event triggers validation (3 events).", testReadableStreamOnReadableEventCount3);
    suite.addTest("Readable event triggers validation (4 events).", testReadableStreamOnReadableEventCount4);
    suite.addTest("Readable event triggers validation (5 events).", testReadableStreamOnReadableEventCount5);
    suite.addTest("Pipe data validation with writable stream.", testPipeDataValidationWithWritableStream);
    suite.addTest("Pipe data validation with multiple writable streams.", testPipeDataValidationAbc);
    suite.addTest("Pipe data validation with numeric data.", testPipeDataValidationNumericData);
    suite.addTest("Pipe data validation with multiple destinations.", testPipeDataValidationMultipleDestinations);
    suite.addTest("Pipe data validation with three destinations.", testPipeDataValidationThreeDestinations);
    suite.addAsyncTest("Writable stream encoding validation for multiple encodings.",
        testWritableStreamWriteMultipleEncodings);
    suite.addAsyncTest("Writable stream end method validation for multiple encodings.",
        testWritableStreamEndMultipleEncodings);
    suite.addAsyncTest("Set default encoding for writable stream with multiple encodings.",
        testWritableStreamSetDefaultEncodingMultipleEncodings);

    return suite.run();
}

const newArr = ['ascii', 'utf-8', 'UTF-8', 'gbk', 'GBK', 'GB2312', 'gb2312',
        'GB18030', 'gb18030', 'ibm866', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13',
        'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u', 'macintosh',
        'windows-874', 'windows-1250', 'windows-1251', 'windows-1252',
        'windows-1253', 'windows-1254', 'windows-1255', 'windows-1256',
        'windows-1257', 'windows-1258', 'big5', 'euc-jp', 'iso-2022-jp',
        'shift_jis', 'euc-kr', 'x-mac-cyrillic', 'utf-16be', 'utf-16le'];

const resArr = ['utf-8', 'utf-8', 'UTF-8', 'gbk', 'GBK', 'GB2312', 'gb2312',
        'GB18030', 'gb18030', 'ibm866', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13',
        'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u', 'macintosh',
        'windows-874', 'windows-1250', 'windows-1251', 'windows-1252',
        'windows-1253', 'windows-1254', 'windows-1255', 'windows-1256',
        'windows-1257', 'windows-1258', 'big5', 'euc-jp', 'iso-2022-jp',
        'shift_jis', 'euc-kr', 'x-mac-cyrillic', 'utf-16be', 'utf-16le'];

async function mustCallCheckAll() {
    return new Promise<void>((resolve, reject) => {
        setTimeout(() => {
            try {
                mustChecks.forEach(check => {
                    check.unsafeCall();
                });
                mustChecks.length = 0;
                resolve(undefined);
            } catch (e) {
                mustChecks.length = 0;
                reject(e)
            }
        }, 2000);
    })
}

function mustCallFn(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (chunk: string, encoding: string, callback: Function) => {
        callCount++;
        fn?.unsafeCall(chunk, encoding, callback);
    }
    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

function mustCallUint8Array(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
        callCount++;
        fn?.unsafeCall(chunk, encoding, callback);
    }
    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

function mustCall(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (err: Error | undefined | null) => {
        callCount++;
        fn?.unsafeCall(err);
    }

    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

let callCount_MyTransform1 = 0;
class MyTransform1 extends stream.Transform {
    constructor() {
        super()
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        callCount_MyTransform1++;
        arktest.assertEQ(encoding, 'gb2312');
        arktest.assertEQ(chunk, '你好123');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding027
async function testTransformStreamWriteEncodingGb2312() {
    mustChecks.push(() => {
        if (callCount_MyTransform1 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_MyTransform1} times.`);
        }
    });

    let transform = new MyTransform1();
    transform.write('你好123', 'gb2312');
    transform.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyReadable1 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding031
function testReadableStreamReadableFlowingState() {
    let readable = new MyReadable1()
    arktest.assertEQ(readable.readableFlowing, null);
}

class MyReadable2 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding032
function testReadableStreamOnReadableEventCount3() {
    let readable = new MyReadable2()
    let readableTest = 0;
    readable.on('readable', () => {
        readableTest++;
    });
    readable.push('test');
    readable.push('test');
    readable.push('test');
    setTimeout(() => {
        arktest.assertEQ(readableTest, 1);
    })
}

class MyReadable3 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding033
function testReadableStreamOnReadableEventCount4() {
    let readable = new MyReadable3()
    let readableTest = 0;
    readable.on('readable', () => {
        readableTest++;
    });
    readable.push('test');
    readable.push('test');
    readable.push('test');
    readable.push('test');
    setTimeout(() => {
        arktest.assertEQ(readableTest, 1);
    })
}

class MyReadable4 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding034
function testReadableStreamOnReadableEventCount5() {
    let readable = new MyReadable4()
    let readableTest = 0;
    readable.on('readable', () => {
        readableTest++;
    });
    readable.push('test');
    readable.push('test');
    readable.push('test');
    readable.push('test');
    readable.push('test');
    setTimeout(() => {
        arktest.assertEQ(readableTest, 1);
    })
}

class MyReadable5 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
        this.push('test');
        this.push(null)
    }
}
class MyWritable5 extends stream.Writable {
    data = ''

    constructor() {
        super()
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.data = (this.data) + JSON.stringify(chunk as Uint8Array);
        callback.unsafeCall()
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding035
function testPipeDataValidationWithWritableStream() {
    const str = "{\"0\":116,\"1\":101,\"2\":115,\"3\":116}";

    let src = new MyReadable5();
    let dest = new MyWritable5();
    src.pipe(dest);
    setTimeout(() => {
        arktest.assertEQ(dest.data, str);
    })
}

class MyReadable6 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
        this.push('abc');
        this.push(null)
    }
}
class MyWritable6 extends stream.Writable {
    data = ''

    constructor() {
        super()
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.data = (this.data) + JSON.stringify(chunk as Uint8Array);
        callback.unsafeCall()
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding036
function testPipeDataValidationAbc() {
    const str = "{\"0\":97,\"1\":98,\"2\":99}";

    let src = new MyReadable6();
    let dest = new MyWritable6();
    src.pipe(dest);
    setTimeout(() => {
        arktest.assertEQ(dest.data, str);
    })
}

class MyReadable7 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
        this.push('123');
        this.push(null)
    }
}
class MyWritable7 extends stream.Writable {
    data = ''

    constructor() {
        super()
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.data = (this.data) + JSON.stringify(chunk as Uint8Array);
        callback.unsafeCall()
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding037
function testPipeDataValidationNumericData() {
    const str = "{\"0\":49,\"1\":50,\"2\":51}";

    let src = new MyReadable7();
    let dest = new MyWritable7();
    src.pipe(dest);
    setTimeout(() => {
        arktest.assertEQ(dest.data, str);
    })
}

class MyReadable8 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
        this.push('123');
        this.push(null)
    }
}
class MyWritable8 extends stream.Writable {
    count: number

    constructor() {
        super()
        this.count = 0
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.count++
        callback.unsafeCall()
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding038
function testPipeDataValidationMultipleDestinations() {

    let src = new MyReadable8();
    let dest1 = new MyWritable8();
    let dest2 = new MyWritable8();
    src.pipe(dest1);
    src.pipe(dest2);
    setTimeout(() => {
        arktest.assertEQ(dest1.count + dest2.count, 2);
    })
}

class MyReadable9 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
        this.push('123');
        this.push(null)
    }
}
class MyWritable9 extends stream.Writable {
    count: number

    constructor() {
        super()
        this.count = 0
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.count++
        callback.unsafeCall()
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding039
function testPipeDataValidationThreeDestinations() {

    let src = new MyReadable9();
    let dest1 = new MyWritable9();
    let dest2 = new MyWritable9();
    let dest3 = new MyWritable9();
    src.pipe(dest1);
    src.pipe(dest2);
    src.pipe(dest3);
    setTimeout(() => {
        arktest.assertEQ(dest1.count + dest2.count + dest3.count, 3);
    })
}

let callCount_MyWrite10 = 0;
class MyWrite10 extends stream.Writable {
    idx = 0;

    constructor(i: int) {
        super();
        this.idx = i;
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite10++;
        arktest.assertEQ(chunk, '测试');
        arktest.assertEQ(encoding, newArr[this.idx]);
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamExtraEncoding001
async function testWritableStreamWriteMultipleEncodings() {
    mustChecks.push(() => {
        if (callCount_MyWrite10 !== 43) {
            throw new Error(`Function was expected to be called 43 times, `
                            + `but was called ${callCount_MyWrite10} times.`);
        }
    });

    for (let i = 0; i < 43; i++) {
        let writable = new MyWrite10(i);
        let result = writable.write('测试', newArr[i]);
        arktest.assertEQ(result, true);
    }
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyWrite11 = 0;
class MyWrite11 extends stream.Writable {
    idx = 0;

    constructor(i: int) {
        super();
        this.idx = i;
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite11++;
        arktest.assertEQ(chunk, '测试');
        arktest.assertEQ(encoding, newArr[this.idx]);
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamExtraEncoding002
async function testWritableStreamEndMultipleEncodings() {
    mustChecks.push(() => {
        if (callCount_MyWrite11 !== 43) {
            throw new Error(`Function was expected to be called 43 times, `
                            + `but was called ${callCount_MyWrite11} times.`);
        }
    });

    for (let i = 0; i < 43; i++) {
        let writable = new MyWrite11(i);
        writable.end('测试', newArr[i], mustCall());
    }
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyWrite12 = 0;
class MyWrite12 extends stream.Writable {
    idx = 0;

    constructor(i: int) {
        super();
        this.idx = i;
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite12++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, resArr[this.idx].toLowerCase());
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamExtraEncoding003
async function testWritableStreamSetDefaultEncodingMultipleEncodings() {
    mustChecks.push(() => {
        if (callCount_MyWrite12 !== 43) {
            throw new Error(`Function was expected to be called 43 times, `
                            + `but was called ${callCount_MyWrite12} times.`);
        }
    });

    for (let i = 0; i < 43; i++) {
        let writable = new MyWrite12(i);
        writable.setDefaultEncoding(newArr[i]);
        writable.write('你好');
    }
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
