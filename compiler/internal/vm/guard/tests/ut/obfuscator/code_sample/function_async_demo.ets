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

async function sleep(time: int): Promise<int> {
    return new Promise<int>((resolve) => {
        setTimeout(()=>{
            resolve(time)
        }, 1000);
    });
}
async function async1() {
    await sleep(100);
    return 'abc';
}
async function async2() {
    return new Promise<string>(
        (resolve) => {
            sleep(100).then(() => {
                resolve('zxc');
            })
        }
    );
}
async function async3() {
    return new Promise<void>(
        (resolve, reject) => {
            sleep(100).then(() => {
                reject(new Error('err'));
            })
        }
    );
}

class Exec {
    public val: string;
    constructor(val: string) {
        this.val = val;
    }

    run() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('exec ' + this.val);
    }
}
async function async4() {
    return new Promise<Exec>(
        (resolve) => {
            sleep(100).then(() => {
                resolve(new Exec('async4'));
            })
        }
    );
}
async function async4Shell(): Promise<Exec> {
    let ret = await async4();
    return ret;
}

async function print() {
    let ret1: string = await async1();
    ret1 += Date.now();

    let ret2: string = await async2();
    ret2 += Date.now();
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log(`ret1${ret1}|ret2${ret2}`);

    try {
        await async3();
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('assert failed');
    } catch (e) {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('err reached: ' + (e as Error).message);
    }

    let ret = await async4Shell();
    ret.run();
}

abstract class AbstractClass {
    public async methodAsync() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('abstract method2');
    }
    abstract methodAbs(): string;
}


class ClassA extends AbstractClass {
    protected method2() {
        this.methodAsync();
        return 'A method2';
    }

    methodAbs() {
        return this.method2();
    }

    print() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log(this.methodAbs());
    }
}

function main() {
    print();

    let cls = new ClassA();
    cls.print();
}
