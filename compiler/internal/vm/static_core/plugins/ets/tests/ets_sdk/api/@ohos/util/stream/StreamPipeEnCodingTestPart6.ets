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

    suite.addAsyncTest("Set default encoding for duplex stream (gbk).", testDuplexStreamSetDefaultEncodingGbk);
    suite.addAsyncTest("Set default encoding for duplex stream (gb2312).", testDuplexStreamSetDefaultEncodingGb2312);
    suite.addAsyncTest("Transform stream encoding validation (gb18030).", testTransformStreamWriteEncodingGb18030);
    suite.addAsyncTest("Transform stream encoding validation (gbk).", testTransformStreamWriteEncodingGbk);

    return suite.run();
}

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

let callCount_MyDuplex1 = 0;
class MyDuplex1 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex1++;
        arktest.assertEQ(encoding, 'gbk');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding023
async function testDuplexStreamSetDefaultEncodingGbk() {
    mustChecks.push(() => {
        if (callCount_MyDuplex1 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex1} times.`);
        }
    });

    let duplex = new MyDuplex1();
    let result = duplex.setDefaultEncoding('gbk');
    duplex.write('你好');
    arktest.assertTrue(result);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyDuplex2 = 0;
class MyDuplex2 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex2++;
        arktest.assertEQ(encoding, 'gb2312');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding024
async function testDuplexStreamSetDefaultEncodingGb2312() {
    mustChecks.push(() => {
        if (callCount_MyDuplex2 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex2} times.`);
        }
    });

    let duplex = new MyDuplex2();
    let result = duplex.setDefaultEncoding('gb2312');
    duplex.write('你好');
    arktest.assertTrue(result);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyTransform3 = 0;
class MyTransform3 extends stream.Transform {
    constructor() {
        super()
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        callCount_MyTransform3++;
        arktest.assertEQ(encoding, 'gb18030');
        arktest.assertEQ(chunk, '你好123');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding025
async function testTransformStreamWriteEncodingGb18030() {
    mustChecks.push(() => {
        if (callCount_MyTransform3 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_MyTransform3} times.`);
        }
    });

    let transform = new MyTransform3();
    transform.write('你好123', 'gb18030');
    transform.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyTransform4 = 0;
class MyTransform4 extends stream.Transform {
    constructor() {
        super()
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        callCount_MyTransform4++;
        arktest.assertEQ(encoding, 'gbk');
        arktest.assertEQ(chunk, '你好123');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding026
async function testTransformStreamWriteEncodingGbk() {
    mustChecks.push(() => {
        if (callCount_MyTransform4 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_MyTransform4} times.`);
        }
    });

    let transform = new MyTransform4();
    transform.write('你好123', 'gbk');
    transform.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
